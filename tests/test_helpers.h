#pragma once

/**
 * test_helpers.h
 *
 * Shared utilities for all MarkIt test files:
 *  - SuppressLogging: a GoogleTest Environment that silences spdlog globally
 *    for the entire test run, so no log files are created and no output
 *    appears on stderr/stdout.
 *  - TempPath: generates a unique, OS temp-dir file path and removes it
 *    automatically when the object goes out of scope (RAII).
 */

#include <atomic>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

// ─────────────────────────────────────────────────────────────────────────────
// SuppressLogging
//
// Register this before RUN_ALL_TESTS() with:
//   ::testing::AddGlobalTestEnvironment(new SuppressLogging());
//
// It sets the spdlog default level to `off` so every spdlog::info/warn/error
// call in the production code becomes a no-op during tests.
// ─────────────────────────────────────────────────────────────────────────────
class SuppressLogging : public ::testing::Environment {
 public:
  void SetUp() override {
    spdlog::set_level(spdlog::level::off);
    // Drop the default logger so Todo::create() / Logger::getInstance()
    // does not try to open a file-sink and fail in a read-only environment.
    spdlog::drop_all();
    // Replace with a null sink-backed logger so any direct spdlog:: calls
    // that bypass the level check still do nothing.
    auto null_logger = std::make_shared<spdlog::logger>("null");
    spdlog::set_default_logger(null_logger);
    spdlog::set_level(spdlog::level::off);
  }
  void TearDown() override { spdlog::drop_all(); }
};

// ─────────────────────────────────────────────────────────────────────────────
// TempPath
//
// Generates a unique path under the OS temp directory. The file is NOT
// created by the constructor; call .path() to get the string. The destructor
// removes the file if it exists.
// ─────────────────────────────────────────────────────────────────────────────
class TempPath {
 public:
  TempPath() {
    static std::atomic<int> counter{0};
    std::filesystem::path dir = std::filesystem::temp_directory_path();
    std::string name =
        "markit_test_" + std::to_string(counter.fetch_add(1)) + ".tmp";
    path_ = (dir / name).string();
  }

  ~TempPath() {
    std::error_code ec;
    std::filesystem::remove(path_, ec);  // silently ignore errors
  }

  const std::string& path() const { return path_; }

  // Implicit conversion so it can be passed directly to APIs taking string&
  operator const std::string&() const { return path_; }

 private:
  std::string path_;
};
