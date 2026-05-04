#pragma once

#include <string>

#include "UpdateChecker.h"
#include <vector>

/**
 * Responsible for downloading a release asset to the local staging directory.
 *
 * Single Responsibility: only downloads — does NOT check versions or replace binaries.
 */
class UpdateDownloader {
 public:
  /**
   * Downloads files from the given URLs to the staging directory.
   * If a file already exists and matches expectedHash, it skips the download.
   * Does NOT extract the archives.
   */
  bool download(const std::vector<UpdateAsset>& assets, bool isFullUpdate);

  /**
   * @return true if a staged binary exists and is ready to apply
   */
  bool isReady() const;

  /**
   * @return Absolute path to the staged (downloaded) binary
   */
  std::string getStagedBinaryPath() const;

  /**
   * Removes any staged update files.
   */
  void cleanup();

 private:
  /**
   * libcurl write callback that writes to a FILE*.
   */
  static size_t writeToFile(void* ptr, size_t size, size_t count, void* stream);
};
