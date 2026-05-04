#pragma once

#include <string>

#include <vector>

struct UpdateAsset {
    std::string downloadUrl;
    std::string assetName;
    std::string expectedHash;
};

/**
 * Holds metadata about an available update.
 * Returned by UpdateChecker::checkForUpdate().
 */
struct UpdateInfo {
  bool updateAvailable = false;
  std::string latestVersion;   // e.g. "0.2.0"
  bool isFullUpdate = true;
  std::vector<UpdateAsset> assetsToDownload; // Either 1 full binary, or multiple patch archives
};

/**
 * Responsible for querying the GitHub Releases API to determine
 * if a newer version of MarkIt is available.
 *
 * Single Responsibility: only checks — does NOT download or apply.
 * Open/Closed: the check logic is self-contained; a different source
 * (e.g. a custom server) would be a new class, not a modification.
 */
class UpdateChecker {
 public:
  /**
   * Queries GitHub for the latest stable release and compares
   * its tag against the current version.
   * @param currentVersion  The running app version (e.g. "0.1.0")
   * @return UpdateInfo with updateAvailable=true if a newer version exists
   */
  UpdateInfo checkForUpdate(const std::string& currentVersion) const;

 private:
  /**
   * Detects the current machine architecture and returns the expected
   * asset name suffix (e.g. "x86_64", "aarch64").
   */
  static std::string detectArch();

  /**
   * Compares two semver strings. Returns true if remote > local.
   * Handles versions like "0.1.0", "1.2.3".
   */
  static bool isNewerVersion(const std::string& local,
                             const std::string& remote);
};
