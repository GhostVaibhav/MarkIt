#pragma once

#include <string>
#include "utils/PathUtils.h"

/**
 * Centralized configuration for the update system.
 * No magic numbers or hardcoded strings elsewhere — all update-related
 * constants live here, following the Single Responsibility Principle.
 */
namespace UpdateConfig {
  // GitHub repository for release lookups
  inline constexpr const char* kRepoOwner = "GhostVaibhav";
  inline constexpr const char* kRepoName  = "MarkIt";

  // GitHub API endpoint (assembled at compile time)
  inline constexpr const char* kLatestReleaseUrl =
      "https://api.github.com/repos/GhostVaibhav/MarkIt/releases/latest";
      
  inline constexpr const char* kReleasesUrl =
      "https://api.github.com/repos/GhostVaibhav/MarkIt/releases";

  // Timing
  inline constexpr int kCheckIntervalMinutes   = 30;
  inline constexpr int kBgSyncIntervalSeconds  = 30;

  // GetPantry rate-limit: 2 requests/second maximum.
  // Enforce a 1000 ms minimum gap AFTER each response (safely under the limit)
  // and wait 3000 ms before retrying after a 429 response.
  inline constexpr int kRequestMinIntervalMs   = 1000;
  inline constexpr int kRateLimitRetryDelayMs  = 3000;

  // Feature flag to bypass the 8-version patch limit for testing
  inline constexpr bool kEnforcePatchLimit = false;

  // Asset naming convention (must match the workflow artifact names)
  // Final URL: .../releases/download/<tag>/<AssetPrefix><arch>.tar.gz
#ifdef _WIN32
  inline constexpr const char* kAssetPrefix = "MarkIt-windows-";
  inline constexpr const char* kPatchPrefix = "patches-windows-";
#else
  inline constexpr const char* kAssetPrefix = "MarkIt-linux-";
  inline constexpr const char* kPatchPrefix = "patches-linux-";
#endif
  inline constexpr const char* kAssetSuffix = ".tar.gz";

  // Updater binary name
#ifdef _WIN32
  inline constexpr const char* kUpdaterBinary = "markit_updater.exe";
#else
  inline constexpr const char* kUpdaterBinary = "markit_updater";
#endif

  // Staging directory for downloaded updates (inside the data path)
  inline std::string getStagingDir() {
    return PathUtils::getDataPath() + "/update";
  }

  // User-Agent for GitHub API requests (required by GitHub)
  inline constexpr const char* kUserAgent = "MarkIt-Updater/1.0";
}
