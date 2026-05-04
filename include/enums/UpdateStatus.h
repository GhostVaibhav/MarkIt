#pragma once

/**
 * Represents the lifecycle states of the auto-update system.
 * Transitions: None → Checking → Available → Downloading → Ready
 *                   └→ UpToDate (no update found)
 *                   └→ Failed (any stage can fail)
 */
enum class UpdateStatus {
  None,         // No check has been performed yet
  Checking,     // Currently querying GitHub API
  UpToDate,     // Latest version matches current version
  Available,    // A newer version exists on GitHub
  Downloading,  // Downloading the new binary in the background
  Ready,        // New binary is staged and ready to apply
  Failed        // An error occurred during check or download
};
