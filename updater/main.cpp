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
#include <array>
#include <queue>
#include "rang/rang.hpp"
#include "json.hpp"
#include "picosha2.h"
#include "config/AppConfig.h"
#include "config/UpdateConfig.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace {

uint64_t readU64LE(const std::vector<uint8_t>& data, size_t offset) {
  uint64_t v = 0;
  for (int i = 0; i < 8; ++i) {
    v |= static_cast<uint64_t>(data[offset + static_cast<size_t>(i)]) << (i * 8);
  }
  return v;
}

struct DecodeNode {
  int left = -1;
  int right = -1;
  int symbol = -1;
};

std::vector<uint8_t> decodeHuffmanPayload(const std::vector<uint8_t>& payload,
                                          const std::array<uint8_t, 256>& lengths,
                                          size_t expectedSize) {
  struct SymbolLen { int sym; uint8_t len; };
  std::vector<SymbolLen> syms;
  for (int s = 0; s < 256; ++s) {
    if (lengths[static_cast<size_t>(s)] > 0) syms.push_back({s, lengths[static_cast<size_t>(s)]});
  }
  if (syms.empty()) return {};

  std::sort(syms.begin(), syms.end(), [](const SymbolLen& a, const SymbolLen& b) {
    if (a.len != b.len) return a.len < b.len;
    return a.sym < b.sym;
  });

  std::array<uint32_t, 256> codes{};
  uint32_t code = 0;
  uint8_t prevLen = syms[0].len;
  for (const auto& s : syms) {
    if (s.len > prevLen) {
      code <<= (s.len - prevLen);
      prevLen = s.len;
    }
    codes[static_cast<size_t>(s.sym)] = code;
    ++code;
  }

  std::vector<DecodeNode> trie(1);
  for (const auto& s : syms) {
    int node = 0;
    for (int bit = static_cast<int>(s.len) - 1; bit >= 0; --bit) {
      const int b = static_cast<int>((codes[static_cast<size_t>(s.sym)] >> bit) & 1u);
      int& next = (b == 0) ? trie[static_cast<size_t>(node)].left : trie[static_cast<size_t>(node)].right;
      if (next < 0) {
        next = static_cast<int>(trie.size());
        trie.push_back(DecodeNode{});
      }
      node = next;
    }
    trie[static_cast<size_t>(node)].symbol = s.sym;
  }

  std::vector<uint8_t> out;
  out.reserve(expectedSize);
  int node = 0;
  for (uint8_t byte : payload) {
    for (int bit = 7; bit >= 0; --bit) {
      const int b = static_cast<int>((byte >> bit) & 1u);
      node = (b == 0) ? trie[static_cast<size_t>(node)].left : trie[static_cast<size_t>(node)].right;
      if (node < 0) throw std::runtime_error("Invalid Huffman stream.");
      if (trie[static_cast<size_t>(node)].symbol >= 0) {
        out.push_back(static_cast<uint8_t>(trie[static_cast<size_t>(node)].symbol));
        node = 0;
        if (out.size() == expectedSize) return out;
      }
    }
  }
  if (out.size() != expectedSize) throw std::runtime_error("Huffman decoded size mismatch.");
  return out;
}

std::vector<uint8_t> decodeRlePayload(const std::vector<uint8_t>& payload, size_t expectedSize) {
  std::vector<uint8_t> out;
  out.reserve(expectedSize);
  size_t i = 0;
  while (i < payload.size()) {
    const uint8_t tag = payload[i++];
    if ((tag & 0x80u) != 0) {
      const size_t runLen = static_cast<size_t>((tag & 0x7Fu) + 3);
      if (i >= payload.size()) throw std::runtime_error("Invalid RLE stream.");
      const uint8_t value = payload[i++];
      out.insert(out.end(), runLen, value);
    } else {
      const size_t litLen = static_cast<size_t>(tag + 1);
      if (i + litLen > payload.size()) throw std::runtime_error("Invalid RLE stream.");
      out.insert(out.end(), payload.begin() + static_cast<std::ptrdiff_t>(i),
                 payload.begin() + static_cast<std::ptrdiff_t>(i + litLen));
      i += litLen;
    }
    if (out.size() > expectedSize) throw std::runtime_error("RLE decoded size overflow.");
  }
  if (out.size() != expectedSize) throw std::runtime_error("RLE decoded size mismatch.");
  return out;
}

uint32_t readU32LE(const std::vector<uint8_t>& data, size_t offset) {
  uint32_t v = 0;
  for (int i = 0; i < 4; ++i) {
    v |= static_cast<uint32_t>(data[offset + static_cast<size_t>(i)]) << (i * 8);
  }
  return v;
}

std::vector<uint8_t> decodeLzPayload(const std::vector<uint8_t>& payload, size_t expectedSize) {
  std::vector<uint8_t> out;
  out.reserve(expectedSize);
  size_t i = 0;
  while (i < payload.size()) {
    const uint8_t tag = payload[i++];
    if ((tag & 0x80u) == 0) {
      const size_t litLen = static_cast<size_t>(tag + 1);
      if (i + litLen > payload.size()) throw std::runtime_error("Invalid LZ literal run.");
      out.insert(out.end(),
                 payload.begin() + static_cast<std::ptrdiff_t>(i),
                 payload.begin() + static_cast<std::ptrdiff_t>(i + litLen));
      i += litLen;
    } else {
      const size_t matchLen = static_cast<size_t>((tag & 0x7Fu) + 3);
      if (i + 4 > payload.size()) throw std::runtime_error("Invalid LZ backref header.");
      const uint32_t dist = readU32LE(payload, i);
      i += 4;
      if (dist == 0 || dist > out.size()) throw std::runtime_error("Invalid LZ backref distance.");
      const size_t start = out.size() - dist;
      for (size_t k = 0; k < matchLen; ++k) {
        out.push_back(out[start + k]);
      }
    }
    if (out.size() > expectedSize) throw std::runtime_error("LZ decoded size overflow.");
  }
  if (out.size() != expectedSize) throw std::runtime_error("LZ decoded size mismatch.");
  return out;
}

std::vector<uint8_t> loadPatchBlob(const fs::path& patchPath) {
  std::ifstream f(patchPath, std::ios::binary);
  if (!f) throw std::runtime_error("Failed to open patches.bin");
  std::vector<uint8_t> fileData((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

  if (fileData.size() < 4) return fileData;
  if (!(fileData[0] == 'M' && fileData[1] == 'K' && fileData[2] == 'P' && fileData[3] == '1')) {
    return fileData; // legacy raw blob
  }
  if (fileData.size() < 4 + 1 + 3 + 8 + 8) throw std::runtime_error("Corrupt compressed patch header.");

  const uint8_t method = fileData[4];
  if (method != 1 && method != 2 && method != 3 && method != 4) {
    throw std::runtime_error("Unsupported patch compression method.");
  }

  const uint64_t rawSize64 = readU64LE(fileData, 8);
  const uint64_t payloadSize64 = readU64LE(fileData, 16);
  const size_t rawSize = static_cast<size_t>(rawSize64);
  const size_t payloadSize = static_cast<size_t>(payloadSize64);

  if (method == 1) {
    const size_t lengthsOff = 24;
    const size_t payloadOff = lengthsOff + 256;
    if (payloadOff + payloadSize > fileData.size()) throw std::runtime_error("Corrupt compressed payload.");
    std::array<uint8_t, 256> lengths{};
    for (size_t i = 0; i < 256; ++i) lengths[i] = fileData[lengthsOff + i];
    std::vector<uint8_t> payload(fileData.begin() + static_cast<std::ptrdiff_t>(payloadOff),
                                 fileData.begin() + static_cast<std::ptrdiff_t>(payloadOff + payloadSize));
    std::vector<uint8_t> raw = decodeHuffmanPayload(payload, lengths, rawSize);
    return raw;
  }

  if (method == 4) {
    const size_t lzSizeOff = 24;
    const size_t lengthsOff = lzSizeOff + 8;
    const size_t payloadOff = lengthsOff + 256;
    if (payloadOff + payloadSize > fileData.size()) throw std::runtime_error("Corrupt compressed payload.");
    const uint64_t lzSize64 = readU64LE(fileData, lzSizeOff);
    const size_t lzSize = static_cast<size_t>(lzSize64);

    std::array<uint8_t, 256> lengths{};
    for (size_t i = 0; i < 256; ++i) lengths[i] = fileData[lengthsOff + i];
    std::vector<uint8_t> payload(fileData.begin() + static_cast<std::ptrdiff_t>(payloadOff),
                                 fileData.begin() + static_cast<std::ptrdiff_t>(payloadOff + payloadSize));
    std::vector<uint8_t> lzDecoded = decodeHuffmanPayload(payload, lengths, lzSize);
    return decodeLzPayload(lzDecoded, rawSize);
  }

  // method == 2 (RLE)
  const size_t genericPayloadOff = 24;
  if (genericPayloadOff + payloadSize > fileData.size()) throw std::runtime_error("Corrupt compressed payload.");
  std::vector<uint8_t> payload(fileData.begin() + static_cast<std::ptrdiff_t>(genericPayloadOff),
                               fileData.begin() + static_cast<std::ptrdiff_t>(genericPayloadOff + payloadSize));
  if (method == 2) {
    return decodeRlePayload(payload, rawSize);
  }
  return decodeLzPayload(payload, rawSize);
}

} // namespace

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

bool g_fastMode = false;

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
    if (!g_fastMode) {
      std::this_thread::sleep_for(std::chrono::milliseconds(sleepPerStep));
    }
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
  bool versionOnly = false;
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
    } else if (std::strcmp(argv[i], "--fast") == 0) {
      g_fastMode = true;
    } else if (std::strcmp(argv[i], "--version") == 0) {
      args.versionOnly = true;
    }
  }
  return args;
}

int main(int argc, char* argv[]) {
  Args args = parseArgs(argc, argv);

  if (args.versionOnly) {
    std::cout << AppConfig{}.version << std::endl;
    return 0;
  }

  std::cout << "markit_updater v" << AppConfig{}.version << " (c) " << UpdateConfig::kRepoOwner << "\n";

  if (args.binaryPath.empty() || args.stagedPath.empty()) {
    std::cerr << "Usage: markit_updater --binary <path> --staged <path> "
                 "[--relaunch]\n";
    return 1;
  }

  // Brief wait for the parent process to fully exit
  if (!g_fastMode) {
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }

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

      fs::path instructionsPath = tempExtractDir / "instructions.bin";
      fs::path patchesBinPath = tempExtractDir / "patches.bin";

      if (fs::exists(instructionsPath) && fs::exists(patchesBinPath)) {
          try {
              std::ifstream i(instructionsPath, std::ios::binary);
              nlohmann::json j = nlohmann::json::from_msgpack(i);
              std::vector<uint8_t> patchBlob = loadPatchBlob(patchesBinPath);

              int fileCount = 0;
              const auto& files = j["files"];
              size_t totalFiles = files.size();

              for (const auto& fileDef : files) {
                  if (!fileDef.is_array() || fileDef.size() < 3) continue;
                  
                  std::string relPathStr = fileDef[0];
                  std::string expectedHash = fileDef[1];
                  
                  fileCount++;
                  if (fileCount % 10 == 0 || fileCount == (int)totalFiles) {
                      int loopPercent = 30 + static_cast<int>((static_cast<float>(fileCount) / totalFiles) * 15.0f);
                      clearScreen();
                      printHeader();
                      printStep("Patching files (" + std::to_string(fileCount) + "/" + std::to_string(totalFiles) + ")...", StepStatus::InProgress);
                      printProgress(loopPercent);
                      std::cout << "    Processing: " << relPathStr << "\n";
                      printWarning();
                  }

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

                  const auto& ops = fileDef[2];
                  if (!ops.is_array()) continue;

                  for (const auto& op : ops) {
                      if (!op.is_array() || op.empty()) continue;
                      int opType = op[0];
                      if (opType == 1) { // "+"
                          if (op.size() < 3) continue;
                          size_t from = op[1];
                          size_t to = op[2];
                          if (to <= patchBlob.size()) {
                              originalData.assign(patchBlob.begin() + static_cast<std::ptrdiff_t>(from),
                                                 patchBlob.begin() + static_cast<std::ptrdiff_t>(to));
                          }
                      } else if (opType == 2) { // "-"
                          originalData.clear();
                      } else if (opType == 4) { // "*-"
                          if (op.size() < 3) continue;
                          size_t start = op[1];
                          size_t end = op[2];
                          if (start <= end && end <= originalData.size()) {
                              originalData.erase(originalData.begin() + start, originalData.begin() + end);
                          }
                      } else if (opType == 3) { // "*+"
                          if (op.size() < 4) continue;
                          size_t start = op[1];
                          size_t from = op[2];
                          size_t to = op[3];
                          if (to <= patchBlob.size() && start <= originalData.size()) {
                              std::vector<uint8_t> chunk(
                                  patchBlob.begin() + static_cast<std::ptrdiff_t>(from),
                                  patchBlob.begin() + static_cast<std::ptrdiff_t>(to));
                              originalData.insert(originalData.begin() + start, chunk.begin(), chunk.end());
                          }
                      }
                  }
                  
                  if (!expectedHash.empty()) {
                      std::vector<unsigned char> hash(picosha2::k_digest_size);
                      picosha2::hash256(originalData.begin(), originalData.end(), hash.begin(), hash.end());
                      std::string computedHash = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
                      
                      if (computedHash != expectedHash) {
                          throw std::runtime_error("Hash mismatch for " + relPathStr + "\nExpected: " + expectedHash + "\nActual:   " + computedHash);
                      }
                  }

                  fs::path tempNewFile = tempExtractDir / "patched_intermediate.tmp";
                  std::ofstream outFile(tempNewFile, std::ios::binary);
                  outFile.write(reinterpret_cast<const char*>(originalData.data()), static_cast<std::streamsize>(originalData.size()));
                  outFile.close();

                  fs::path destActive = activeStateDir / relPath;
                  fs::create_directories(destActive.parent_path());
                  fs::copy_file(tempNewFile, destActive, fs::copy_options::overwrite_existing);
              }
          } catch (const std::exception& e) {
              showFinalScreen("Updating...", StepStatus::Failed, 30, std::string("Patch failed: ") + e.what());
              std::cout << "Press Enter to exit...";
              if (!g_fastMode) std::cin.get();
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

  if (!g_fastMode) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // ── Relaunch ──
  if (args.relaunch) {
    if (!g_fastMode) std::this_thread::sleep_for(std::chrono::milliseconds(800));
    std::cout << "\n  Restarting MarkIt...\n\n";
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
