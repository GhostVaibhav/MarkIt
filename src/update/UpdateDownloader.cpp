#define _CRT_SECURE_NO_WARNINGS
#include "UpdateDownloader.h"

#include <curl/curl.h>
#include <spdlog/spdlog.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>

#include "config/UpdateConfig.h"
#include "picosha2.h"

#include <fstream>

bool UpdateDownloader::download(const std::vector<UpdateAsset>& assets, bool isFullUpdate) {
  try {
    std::string stagingDir = UpdateConfig::getStagingDir();
    
    // Always clear the staging dir before starting a new download batch to avoid mixing patches from failed updates
    if (std::filesystem::exists(stagingDir)) {
      std::filesystem::remove_all(stagingDir);
    }
    std::filesystem::create_directories(stagingDir);

    for (const auto& asset : assets) {
        std::string archivePath = (std::filesystem::path(stagingDir) / asset.assetName).string();

        // Download the archive
        CURL* curl = curl_easy_init();
        if (!curl) {
          spdlog::error("UpdateDownloader: Failed to initialize curl");
          return false;
        }

        FILE* fp = fopen(archivePath.c_str(), "wb");
        if (!fp) {
          spdlog::error("UpdateDownloader: Cannot create file {}", archivePath);
          curl_easy_cleanup(curl);
          return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, asset.downloadUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToFile);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, UpdateConfig::kUserAgent);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

        CURLcode res = curl_easy_perform(curl);
        fclose(fp);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
          spdlog::error("UpdateDownloader: Download failed for {}: {}",
                        asset.assetName, curl_easy_strerror(res));
          std::filesystem::remove(archivePath);
          return false;
        }

        // Verify Hash after download
        if (!asset.expectedHash.empty()) {
            std::ifstream f(archivePath, std::ios::binary);
            if (f.is_open()) {
              std::vector<unsigned char> hash(picosha2::k_digest_size);
              picosha2::hash256(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>(), hash.begin(), hash.end());
              std::string computedHash = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
              if (computedHash != asset.expectedHash) {
                spdlog::error("UpdateDownloader: Hash mismatch for {}", asset.assetName);
                return false;
              }
            }
        }

        spdlog::info("UpdateDownloader: Downloaded {} to {}", asset.assetName,
                     archivePath);
    }

    return true;

  } catch (const std::exception& e) {
    spdlog::error("UpdateDownloader: Exception during download: {}", e.what());
    return false;
  }
}

bool UpdateDownloader::isReady() const {
  std::string stagingDir = UpdateConfig::getStagingDir();
  if (!std::filesystem::exists(stagingDir)) return false;

  for (const auto& entry : std::filesystem::directory_iterator(stagingDir)) {
    if (entry.is_regular_file()) return true;
  }
  return false;
}

std::string UpdateDownloader::getStagedBinaryPath() const {
  std::string stagingDir = UpdateConfig::getStagingDir();
  if (!std::filesystem::exists(stagingDir)) return "";

  for (const auto& entry : std::filesystem::directory_iterator(stagingDir)) {
    if (entry.is_regular_file()) {
      return entry.path().string();
    }
  }
  return "";
}

void UpdateDownloader::cleanup() {
  std::string stagingDir = UpdateConfig::getStagingDir();
  if (std::filesystem::exists(stagingDir)) {
    std::filesystem::remove_all(stagingDir);
    spdlog::info("UpdateDownloader: Cleaned up staging directory");
  }
}

size_t UpdateDownloader::writeToFile(void* ptr, size_t size, size_t count,
                                     void* stream) {
  return fwrite(ptr, size, count, static_cast<FILE*>(stream));
}
