#include "FileManager.h"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

FileManager::FileManager(std::string filename)
    : filename(std::move(filename)) {}

std::optional<std::string> FileManager::readFile() const {
  if (!std::filesystem::exists(filename)) {
    return std::nullopt;
  }

  std::ifstream file(filename);
  if (!file.is_open()) {
    spdlog::error("FileManager: Failed to open file for reading: {}", filename);
    return std::nullopt;
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  spdlog::info("FileManager: Read file successfully: {} ({} bytes)", filename,
               content.size());
  return content;
}

bool FileManager::writeFile(const std::string& content) const {
  std::ofstream file(filename, std::ios::out | std::ios::trunc);
  if (!file.is_open()) {
    spdlog::error("FileManager: Failed to open file for writing: {}", filename);
    return false;
  }

  file << content;
  spdlog::info("FileManager: Wrote file successfully: {} ({} bytes)", filename,
               content.size());
  return true;
}

bool FileManager::deleteFile() const {
  if (!std::filesystem::exists(filename)) {
    spdlog::info("FileManager: Delete skipped, file does not exist: {}",
                 filename);
    return true;
  }

  std::error_code ec;
  bool success = std::filesystem::remove(filename, ec) && !ec;
  if (success) {
    spdlog::info("FileManager: Deleted file successfully: {}", filename);
  } else {
    spdlog::error("FileManager: Failed to delete file: {} - {}", filename,
                  ec.message());
  }
  return success;
}
