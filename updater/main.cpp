/**
 * markit_updater — External binary updater for MarkIt
 *
 * Launched by the main MarkIt app when a staged update is ready.
 * Replaces the current binary with the new one and optionally relaunches.
 *
 * Usage: markit_updater --binary <path> --staged <path> [--relaunch]
 *
 * No external dependencies — uses only C++17 standard library + POSIX/Win32.
 */

#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <fstream>
#include "rang/rang.hpp"
#include "json.hpp"
#include "picosha2.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

// ─── Terminal UI Helpers ────────────────────────────────────────────

enum class StepStatus {
  InProgress,
  Success,
  Failed
};

void clearScreen() {
  std::cout << "\033[2J\033[H" << std::flush;
}

void printHeader() {
  std::cout << "\n";
  std::cout << rang::style::bold << rang::fg::cyan;
  std::cout << "  MarkIt Updater\n";
  std::cout << "  --------------\n\n";
  std::cout << rang::style::reset;
}

void printStep(const std::string& message, StepStatus status) {
  std::cout << "  ";
  switch (status) {
    case StepStatus::InProgress:
      std::cout << rang::fg::yellow << "> " << rang::style::reset;
      break;
    case StepStatus::Success:
      std::cout << rang::fg::green << "+ " << rang::style::reset;
      break;
    case StepStatus::Failed:
      std::cout << rang::fg::red << "x " << rang::style::reset;
      break;
  }
  std::cout << message << "\n";
}

void printProgress(int percent) {
  std::cout << "    ";

  constexpr int barWidth = 30;
  int filled = (barWidth * percent) / 100;

  std::cout << rang::style::dim << "[" << rang::style::reset;
  for (int i = 0; i < barWidth; ++i) {
    if (i < filled) {
      std::cout << rang::fg::cyan << "#" << rang::style::reset;
    } else {
      std::cout << rang::style::dim << "-" << rang::style::reset;
    }
  }
  std::cout << rang::style::dim << "] " << rang::style::reset;
  std::cout << percent << "%\n";
}

void printWarning() {
  std::cout << "\n  " << rang::style::dim << "(Do not close this window)" << rang::style::reset << "\n";
}

void animatedRender(const std::string& step, int fromPercent, int toPercent,
                    int durationMs = 500) {
  int steps = toPercent - fromPercent;
  int sleepPerStep = (steps > 0) ? (durationMs / steps) : durationMs;

  for (int p = fromPercent; p <= toPercent; ++p) {
    clearScreen();
    printHeader();
    printStep(step, StepStatus::InProgress);
    printProgress(p);
    printWarning();
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepPerStep));
  }
}

void showFinalScreen(const std::string& step, StepStatus status, int percent,
                     const std::string& message) {
  clearScreen();
  printHeader();
  printStep(step, status);
  printProgress(percent);
  
  std::cout << "\n  ";
  if (status == StepStatus::Failed) std::cout << rang::fg::red;
  else if (status == StepStatus::Success) std::cout << rang::fg::green;
  
  std::cout << message << rang::style::reset << "\n\n";
}

// ─── Main Logic ─────────────────────────────────────────────────────

struct Args {
  std::string binaryPath;
  std::string stagedPath;
  bool relaunch = false;
};

Args parseArgs(int argc, char* argv[]) {
  Args args;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--binary") == 0 && i + 1 < argc) {
      args.binaryPath = argv[++i];
    } else if (std::strcmp(argv[i], "--staged") == 0 && i + 1 < argc) {
      args.stagedPath = argv[++i];
    } else if (std::strcmp(argv[i], "--relaunch") == 0) {
      args.relaunch = true;
    }
  }
  return args;
}

int main(int argc, char* argv[]) {
  Args args = parseArgs(argc, argv);

  if (args.binaryPath.empty() || args.stagedPath.empty()) {
    std::cerr << "Usage: markit_updater --binary <path> --staged <path> "
                 "[--relaunch]\n";
    return 1;
  }

  // Brief wait for the parent process to fully exit
  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  // ── Step 1: Preparing ──
  animatedRender("Preparing update...", 0, 10, 400);

  if (!fs::exists(args.stagedPath)) {
    showFinalScreen("Preparing update...", StepStatus::Failed, 10,
                    "Staged archive not found!");
    return 1;
  }

  // ── Step 2: Extracting ──
  animatedRender("Extracting update...", 10, 30, 800);

  fs::path stagingDir = fs::path(args.stagedPath);
  fs::path tempExtractDir = stagingDir / "temp_extract";
  
  try {
    if (fs::exists(tempExtractDir)) {
      fs::remove_all(tempExtractDir);
    }
    fs::create_directories(tempExtractDir);
  } catch (const std::exception& e) {
    showFinalScreen("Extracting...", StepStatus::Failed, 30,
                    std::string("Extraction setup failed: ") + e.what());
    return 1;
  }

  // Collect and sort tar.gz files
  std::vector<std::string> archiveFiles;
  for (const auto& entry : fs::directory_iterator(stagingDir)) {
      if (entry.is_regular_file() && entry.path().extension() == ".gz") {
          archiveFiles.push_back(entry.path().string());
      }
  }
  std::sort(archiveFiles.begin(), archiveFiles.end());

  if (archiveFiles.empty()) {
      showFinalScreen("Updating...", StepStatus::Failed, 30, "No update archives found!");
      return 1;
  }

  fs::path installDir = fs::path(args.binaryPath).parent_path();
  fs::path activeStateDir = stagingDir / "active_state";
  fs::create_directories(activeStateDir);

  for (const auto& archivePath : archiveFiles) {
      // Clear temp extract dir for each archive
      for (const auto& entry : fs::directory_iterator(tempExtractDir)) {
          fs::remove_all(entry.path());
      }

      std::string extractCmd = "tar xzf \"" + archivePath + "\" -C \"" + tempExtractDir.string() + "\"";
      int ret = system(extractCmd.c_str());
      if (ret != 0) {
          showFinalScreen("Updating...", StepStatus::Failed, 30, "Extraction failed!");
          return 1;
      }

      fs::path instructionsPath = tempExtractDir / "instructions.json";
      fs::path patchesBinPath = tempExtractDir / "patches.bin";

      if (fs::exists(instructionsPath) && fs::exists(patchesBinPath)) {
          // It's a patch update
          try {
              std::ifstream i(instructionsPath);
              nlohmann::json j; i >> j;
              std::ifstream binData(patchesBinPath, std::ios::binary);

              for (const auto& fileDef : j["files"]) {
                  std::string expectedHash = fileDef.value("hash", "");
                  std::string relPathStr = fileDef["path"];
                  fs::path relPath = fs::path(relPathStr);
                  
                  fs::path currentState = activeStateDir / relPath;
                  if (!fs::exists(currentState)) {
                      currentState = installDir / relPath;
                  }
                  
                  std::vector<uint8_t> originalData;
                  if (fs::exists(currentState)) {
                      std::ifstream origFile(currentState, std::ios::binary);
                      originalData.assign((std::istreambuf_iterator<char>(origFile)), std::istreambuf_iterator<char>());
                  }

                  for (const auto& op : fileDef["operations"]) {
                      std::string opType = op["op"];
                      if (opType == "+") {
                          size_t from = op["from_bytes"];
                          size_t to = op["to_bytes"];
                          size_t size = to - from;
                          binData.seekg(from);
                          originalData.resize(size);
                          binData.read(reinterpret_cast<char*>(originalData.data()), size);
                      } else if (opType == "-") {
                          originalData.clear();
                      } else if (opType == "*-") {
                          size_t start = op["start_bytes"];
                          size_t end = op["end_bytes"];
                          if (start <= end && end <= originalData.size()) {
                              originalData.erase(originalData.begin() + start, originalData.begin() + end);
                          }
                      } else if (opType == "*+") {
                          size_t start = op["start_bytes"];
                          size_t from = op["from_bytes"];
                          size_t to = op["to_bytes"];
                          size_t size = to - from;
                          std::vector<uint8_t> chunk(size);
                          binData.seekg(from);
                          binData.read(reinterpret_cast<char*>(chunk.data()), size);
                          if (start <= originalData.size()) {
                              originalData.insert(originalData.begin() + start, chunk.begin(), chunk.end());
                          }
                      }
                  }
                  
                  std::vector<unsigned char> hash(picosha2::k_digest_size);
                  picosha2::hash256(originalData.begin(), originalData.end(), hash.begin(), hash.end());
                  std::string computedHash = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
                  
                  if (!expectedHash.empty() && computedHash != expectedHash) {
                      throw std::runtime_error("Patch verification failed! Hash mismatch for " + relPathStr);
                  }

                  fs::path tempNewFile = tempExtractDir / "patched_intermediate.tmp";
                  std::ofstream outFile(tempNewFile, std::ios::binary);
                  outFile.write(reinterpret_cast<const char*>(originalData.data()), originalData.size());
                  outFile.close();

                  fs::create_directories((activeStateDir / relPath).parent_path());
                  fs::copy_file(tempNewFile, activeStateDir / relPath, fs::copy_options::overwrite_existing);
              }
          } catch (const std::exception& e) {
              showFinalScreen("Updating...", StepStatus::Failed, 30, std::string("Patch failed: ") + e.what());
              return 1;
          }
      } else {
          // Standard full binary replacement
          for (const auto& entry : fs::recursive_directory_iterator(tempExtractDir)) {
              if (entry.is_regular_file()) {
                  fs::path relPath = fs::relative(entry.path(), tempExtractDir);
                  fs::create_directories((activeStateDir / relPath).parent_path());
                  fs::copy_file(entry.path(), activeStateDir / relPath, fs::copy_options::overwrite_existing);
              }
          }
      }
  }

  // ── Step 4: Replace ──
  animatedRender("Installing new version...", 45, 75, 500);

  try {
      for (const auto& entry : fs::recursive_directory_iterator(activeStateDir)) {
          if (!entry.is_regular_file()) continue;
          
          fs::path relPath = fs::relative(entry.path(), activeStateDir);
          fs::path destPath = installDir / relPath;
          
          std::string filename = relPath.filename().string();
          if (filename == "markit_updater.exe" || filename == "markit_updater") {
              destPath = installDir / (filename + ".new");
          }
          
          fs::create_directories(destPath.parent_path());
          fs::copy_file(entry.path(), destPath, fs::copy_options::overwrite_existing);
      }
  } catch (const std::exception& e) {
    showFinalScreen("Installing...", StepStatus::Failed, 75,
                    std::string("Install failed: ") + e.what());
    return 1;
  }

  // ── Step 5: Permissions ──
  animatedRender("Setting permissions...", 75, 85, 200);

#ifndef _WIN32
  chmod(args.binaryPath.c_str(), 0755);
  // Also chmod the updater if we dropped it
  fs::path updaterNew = installDir / "markit_updater.new";
  if (fs::exists(updaterNew)) {
      chmod(updaterNew.string().c_str(), 0755);
  }
#endif

  // ── Step 6: Cleanup ──
  animatedRender("Cleaning up...", 85, 95, 300);

  try {
    fs::remove_all(stagingDir);
  } catch (...) {
    // Non-critical — cleanup failures are acceptable
  }

  // ── Done ──
  showFinalScreen("Update complete!", StepStatus::Success, 100,
                  "Restarting MarkIt...");

  std::this_thread::sleep_for(std::chrono::seconds(1));

  // ── Relaunch ──
  if (args.relaunch) {
#ifdef _WIN32
    std::string cmd = "\"" + args.binaryPath + "\"";
    system(cmd.c_str());
#else
    execl(args.binaryPath.c_str(), args.binaryPath.c_str(), nullptr);
    // If exec fails, fall through
    std::cerr << "Failed to relaunch MarkIt\n";
    return 1;
#endif
  } else {
    std::cout << "\n  Run '" << rang::style::bold << args.binaryPath << rang::style::reset
              << "' to start MarkIt.\n\n";
  }

  return 0;
}
