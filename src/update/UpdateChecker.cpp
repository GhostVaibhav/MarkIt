#include "UpdateChecker.h"

#include <spdlog/spdlog.h>
#include "json.hpp"

#include <sstream>
#include <vector>

#include <curl/curl.h>
#include "config/UpdateConfig.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/utsname.h>
#endif

namespace {
size_t writeCallback(void* ptr, size_t size, size_t count, void* stream) {
  auto* str = static_cast<std::string*>(stream);
  str->append(static_cast<char*>(ptr), size * count);
  return size * count;
}
} // namespace

UpdateInfo UpdateChecker::checkForUpdate(
    const std::string& currentVersion) const {
  UpdateInfo info;
  try {
    CURL* curl = curl_easy_init();
    if (!curl) {
      spdlog::error("UpdateChecker: Failed to init curl");
      return info;
    }

    std::string responseBody;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/vnd.github+json");

    curl_easy_setopt(curl, CURLOPT_URL, UpdateConfig::kReleasesUrl);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, UpdateConfig::kUserAgent);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
      spdlog::error("UpdateChecker: curl failed: {}", curl_easy_strerror(res));
      return info;
    }

    if (responseBody.empty()) {
      spdlog::warn("UpdateChecker: Empty response from GitHub API");
      return info;
    }

    auto json = nlohmann::json::parse(responseBody);
    if (!json.is_array()) {
        spdlog::warn("UpdateChecker: GitHub API did not return an array of releases");
        return info;
    }

    // Filter to stable releases
    std::vector<nlohmann::json> stableReleases;
    for (const auto& release : json) {
        if (release.contains("prerelease") && release["prerelease"].get<bool>()) continue;
        std::string tagName = release.value("tag_name", "");
        if (tagName.empty()) continue;
        stableReleases.push_back(release);
    }

    if (stableReleases.empty()) {
        spdlog::info("UpdateChecker: No stable releases found");
        return info;
    }

    // Latest is the first one
    std::string latestTagName = stableReleases[0].value("tag_name", "");
    std::string remoteVersion = latestTagName;
    if (!remoteVersion.empty() && remoteVersion[0] == 'v') {
      remoteVersion = remoteVersion.substr(1);
    }

    info.latestVersion = remoteVersion;

    if (!isNewerVersion(currentVersion, remoteVersion)) {
      spdlog::info("UpdateChecker: Current version {} is up to date (latest: {})",
                   currentVersion, remoteVersion);
      return info;
    }

    // Find the current version index
    int currentIndex = -1;
    for (size_t i = 0; i < stableReleases.size(); ++i) {
        std::string tag = stableReleases[i].value("tag_name", "");
        if (!tag.empty() && tag[0] == 'v') tag = tag.substr(1);
        if (tag == currentVersion) {
            currentIndex = static_cast<int>(i);
            break;
        }
    }

    std::string arch = detectArch();
    std::string expectedFullAsset = std::string(UpdateConfig::kAssetPrefix) + arch + std::string(UpdateConfig::kAssetSuffix);
    std::string expectedPatchAsset = std::string(UpdateConfig::kPatchPrefix) + arch + std::string(UpdateConfig::kAssetSuffix);

    bool usePatch = false;
    bool directPatch = false;
    if (currentIndex != -1) {
        spdlog::info("Current Index != -1");
        if (UpdateConfig::kEnforcePatchLimit && currentIndex > 8) {
            usePatch = false;
            spdlog::info("UpdateChecker: Exceeded patch limit ({} > 8), falling back to full download", currentIndex);
        } else {
            usePatch = true;
            spdlog::info("Scanning for patches");
            // Verify all intermediate releases have the patch asset
            for (int i = currentIndex - 1; i >= 0; --i) {
                bool foundPatch = false;
                if (stableReleases[i].contains("assets") && stableReleases[i]["assets"].is_array()) {
                    for (const auto& asset : stableReleases[i]["assets"]) {
                        if (asset.value("name", "") == expectedPatchAsset) {
                            foundPatch = true;
                            break;
                        }
                    }
                }
                if (!foundPatch) {
                    spdlog::warn("UpdateChecker: Missing patch asset in release {}", stableReleases[i].value("tag_name", ""));
                    usePatch = false;
                    break;
                }
            }
        }
    } else {
        spdlog::info("Current version {} not found in releases. Checking if latest release {} has a direct patch.", currentVersion, remoteVersion);
        if (stableReleases[0].contains("assets") && stableReleases[0]["assets"].is_array()) {
            for (const auto& asset : stableReleases[0]["assets"]) {
                if (asset.value("name", "") == expectedPatchAsset) {
                    spdlog::info("Found direct patch in latest release.");
                    directPatch = true;
                    usePatch = true;
                    break;
                }
            }
        }
    }

    if (usePatch) {
        spdlog::info("Using patch chain for update");
        info.isFullUpdate = false;
        info.updateAvailable = true;
        
        if (directPatch) {
            for (const auto& asset : stableReleases[0]["assets"]) {
                if (asset.value("name", "") == expectedPatchAsset) {
                    UpdateAsset ua;
                    ua.assetName = asset.value("name", "") + "_seq1.tar.gz";
                    ua.downloadUrl = asset.value("browser_download_url", "");
                    std::string digest = asset.value("digest", "");
                    if (digest.find("sha256:") == 0) ua.expectedHash = digest.substr(7);
                    else ua.expectedHash = digest;
                    info.assetsToDownload.push_back(ua);
                    break;
                }
            }
        } else {
            // The array is sorted newest first. 
            // We want to apply patches from oldest to newest! So loop from current-1 down to 0
            for (int i = currentIndex - 1; i >= 0; --i) {
                if (stableReleases[i].contains("assets") && stableReleases[i]["assets"].is_array()) {
                    for (const auto& asset : stableReleases[i]["assets"]) {
                        if (asset.value("name", "") == expectedPatchAsset) {
                            UpdateAsset ua;
                            int patchSeq = currentIndex - i;
                            ua.assetName = asset.value("name", "") + "_seq" + std::to_string(patchSeq) + ".tar.gz"; // Uniquify the patch filename
                            ua.downloadUrl = asset.value("browser_download_url", "");
                            std::string digest = asset.value("digest", "");
                            if (digest.find("sha256:") == 0) ua.expectedHash = digest.substr(7);
                            else ua.expectedHash = digest;
                            info.assetsToDownload.push_back(ua);
                            break;
                        }
                    }
                }
            }
        }
        spdlog::info("UpdateChecker: Patch chain available! {} -> {} ({} patches)", currentVersion, remoteVersion, info.assetsToDownload.size());
        return info;
    }

    // Fallback to full download
    info.isFullUpdate = true;
    if (stableReleases[0].contains("assets") && stableReleases[0]["assets"].is_array()) {
        for (const auto& asset : stableReleases[0]["assets"]) {
            if (asset.value("name", "") == expectedFullAsset) {
                info.updateAvailable = true;
                UpdateAsset ua;
                ua.assetName = asset.value("name", "");
                ua.downloadUrl = asset.value("browser_download_url", "");
                std::string digest = asset.value("digest", "");
                if (digest.find("sha256:") == 0) ua.expectedHash = digest.substr(7);
                else ua.expectedHash = digest;
                info.assetsToDownload.push_back(ua);
                
                spdlog::info("UpdateChecker: Full update available! {} -> {} (asset: {})", currentVersion, remoteVersion, ua.assetName);
                return info;
            }
        }
    }

    spdlog::warn("UpdateChecker: Update {} exists but no full asset found for arch '{}'", remoteVersion, arch);

  } catch (const std::exception& e) {
    spdlog::error("UpdateChecker: Failed to check for update: {}", e.what());
  }

  return info;
}

std::string UpdateChecker::detectArch() {
#ifdef _WIN32
  SYSTEM_INFO sysInfo;
  GetNativeSystemInfo(&sysInfo);
  switch (sysInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64: return "x86_64";
    case PROCESSOR_ARCHITECTURE_ARM64: return "aarch64";
    case PROCESSOR_ARCHITECTURE_ARM:   return "armv7";
    case PROCESSOR_ARCHITECTURE_INTEL: return "i686";
    default: return "unknown";
  }
#else
  struct utsname buf;
  if (uname(&buf) == 0) {
    std::string machine(buf.machine);
    if (machine == "x86_64" || machine == "amd64") return "x86_64";
    if (machine == "aarch64" || machine == "arm64") return "aarch64";
    if (machine.substr(0, 5) == "armv7") return "armv7";
    if (machine == "i686" || machine == "i386") return "i686";
    if (machine == "riscv64") return "riscv64";
    if (machine == "ppc64le") return "ppc64le";
    if (machine == "s390x") return "s390x";
    return machine;
  }
  return "unknown";
#endif
}

bool UpdateChecker::isNewerVersion(const std::string& local,
                                   const std::string& remote) {
  auto parse = [](const std::string& v) -> std::vector<int> {
    std::vector<int> parts;
    std::istringstream ss(v);
    std::string token;
    while (std::getline(ss, token, '.')) {
      try {
        parts.push_back(std::stoi(token));
      } catch (...) {
        parts.push_back(0);
      }
    }
    // Pad to 3 components (major.minor.patch)
    while (parts.size() < 3) parts.push_back(0);
    return parts;
  };

  auto lv = parse(local);
  auto rv = parse(remote);

  for (size_t i = 0; i < 3; ++i) {
    if (rv[i] > lv[i]) return true;
    if (rv[i] < lv[i]) return false;
  }
  return false;  // Equal versions
}
