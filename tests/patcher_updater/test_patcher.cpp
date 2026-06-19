/**
 * test_patcher.cpp
 *
 * Integration tests for DiffEngine — compiled directly as a C++ library.
 * No subprocess is used; DiffEngine::generatePatches() is called in-process.
 *
 * Each test fixture:
 *  1. Creates a fresh temp output directory.
 *  2. Calls DiffEngine(old_dir, new_dir, out_dir).generatePatches().
 *  3. Reads and parses instructions.bin (MessagePack) to inspect the file list.
 *  4. Checks patches.bin existence / size.
 *
 * Fixture layout (relative to FIXTURES_DIR):
 *   v1/ → v2/ : adds files, modifies binary + config
 *   v2/ → v3/ : deletes banner.txt, config unchanged, logo shrinks
 *   v3/ → v4/ : large text diff (changelog completely rewritten)
 *   v1/ → v1/ : identical dirs → empty instructions
 */

#include <atomic>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "DiffEngine.h"
#include "json.hpp"
#include "spdlog/spdlog.h"

namespace fs = std::filesystem;

// ─── Global logging suppression ─────────────────────────────────────────────

class SuppressPatcherLogging : public ::testing::Environment {
 public:
  void SetUp() override {
    spdlog::set_level(spdlog::level::off);
    spdlog::drop_all();
    auto null_logger = std::make_shared<spdlog::logger>("null");
    spdlog::set_default_logger(null_logger);
    spdlog::set_level(spdlog::level::off);
  }
  void TearDown() override { spdlog::drop_all(); }
};

// ─── Helpers ────────────────────────────────────────────────────────────────

static std::atomic<int> tempCounter{0};

// RAII temp directory — created on construction, removed on destruction.
class TempDir {
 public:
  TempDir() {
    path_ = fs::temp_directory_path() /
            ("markit_ptest_" + std::to_string(tempCounter.fetch_add(1)));
    fs::create_directories(path_);
  }
  ~TempDir() {
    std::error_code ec;
    fs::remove_all(path_, ec);
  }
  const fs::path& path() const { return path_; }
  std::string str() const { return path_.string(); }

 private:
  fs::path path_;
};

// Parse instructions.bin (MessagePack) and return the "files" array.
static nlohmann::json parseInstructions(const fs::path& outDir) {
  fs::path p = outDir / "instructions.bin";
  std::ifstream f(p, std::ios::binary);
  if (!f) return nlohmann::json::array();
  nlohmann::json j = nlohmann::json::from_msgpack(f);
  return j.value("files", nlohmann::json::array());
}

// Return the relative path strings of all files in the instructions.
static std::vector<std::string> instructionPaths(const nlohmann::json& files) {
  std::vector<std::string> paths;
  for (const auto& entry : files) {
    if (entry.is_array() && !entry.empty()) {
      paths.push_back(entry[0].get<std::string>());
    }
  }
  return paths;
}

// Return the first op-type integer for a given rel-path in the instructions,
// or -1 if not found.
static int firstOpType(const nlohmann::json& files, const std::string& relPath) {
  for (const auto& entry : files) {
    if (!entry.is_array() || entry.size() < 3) continue;
    if (entry[0].get<std::string>() == relPath) {
      const auto& ops = entry[2];
      if (ops.is_array() && !ops.empty()) {
        const auto& firstOp = ops[0];
        if (firstOp.is_array() && !firstOp.empty()) {
          return firstOp[0].get<int>();
        }
      }
    }
  }
  return -1;
}

// True if relPath appears anywhere in the instructions files list.
static bool isInInstructions(const nlohmann::json& files,
                              const std::string& relPath) {
  for (const auto& entry : files) {
    if (entry.is_array() && !entry.empty() &&
        entry[0].get<std::string>() == relPath)
      return true;
  }
  return false;
}

static std::string fixDir(const std::string& version) {
  return std::string(FIXTURES_DIR) + "/" + version;
}

// ─── Fixture ────────────────────────────────────────────────────────────────

class PatcherTest : public ::testing::Test {
 protected:
  TempDir outDir;
};

// ─── Tests: v1 → v2 ─────────────────────────────────────────────────────────

TEST_F(PatcherTest, V1toV2_GeneratePatches_BothOutputFilesExist) {
  patcher::DiffEngine engine(fixDir("v1"), fixDir("v2"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  EXPECT_TRUE(fs::exists(outDir.path() / "patches.bin"));
  EXPECT_TRUE(fs::exists(outDir.path() / "instructions.bin"));
}

TEST_F(PatcherTest, V1toV2_GeneratePatches_PatchesBinIsNonEmpty) {
  patcher::DiffEngine engine(fixDir("v1"), fixDir("v2"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  // patches.bin may be zero-size only if everything is a net-new tiny file;
  // with our fixtures it must have content.
  EXPECT_GT(fs::file_size(outDir.path() / "patches.bin"), 0u);
}

TEST_F(PatcherTest, V1toV2_NewFile_Banner_HasPlusOp) {
  patcher::DiffEngine engine(fixDir("v1"), fixDir("v2"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  auto files = parseInstructions(outDir.path());
  // banner.txt is new in v2 → op type 1 ("+")
  EXPECT_EQ(firstOpType(files, "assets/banner.txt"), 1)
      << "Expected assets/banner.txt to have op '+' (1)";
}

TEST_F(PatcherTest, V1toV2_NewFile_Changelog_HasPlusOp) {
  patcher::DiffEngine engine(fixDir("v1"), fixDir("v2"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  auto files = parseInstructions(outDir.path());
  EXPECT_EQ(firstOpType(files, "changelog.txt"), 1)
      << "Expected changelog.txt to have op '+' (1)";
}

TEST_F(PatcherTest, V1toV2_ModifiedFile_Binary_PresentInInstructions) {
  patcher::DiffEngine engine(fixDir("v1"), fixDir("v2"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  auto files = parseInstructions(outDir.path());
  EXPECT_TRUE(isInInstructions(files, "markit_binary"))
      << "markit_binary was modified and must appear in instructions";
}

TEST_F(PatcherTest, V1toV2_ModifiedFile_Config_PresentInInstructions) {
  patcher::DiffEngine engine(fixDir("v1"), fixDir("v2"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  auto files = parseInstructions(outDir.path());
  EXPECT_TRUE(isInInstructions(files, "app_config.cfg"));
}

// ─── Tests: v2 → v3 ─────────────────────────────────────────────────────────

TEST_F(PatcherTest, V2toV3_DeletedFile_Banner_HasMinusOp) {
  patcher::DiffEngine engine(fixDir("v2"), fixDir("v3"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  auto files = parseInstructions(outDir.path());
  // banner.txt is deleted in v3 → op type 2 ("-")
  EXPECT_EQ(firstOpType(files, "assets/banner.txt"), 2)
      << "Expected assets/banner.txt to have op '-' (2) in v2→v3";
}

TEST_F(PatcherTest, V2toV3_UnchangedFile_Config_AbsentFromInstructions) {
  patcher::DiffEngine engine(fixDir("v2"), fixDir("v3"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  auto files = parseInstructions(outDir.path());
  // app_config.cfg is byte-identical in v2 and v3 — must NOT appear
  EXPECT_FALSE(isInInstructions(files, "app_config.cfg"))
      << "app_config.cfg is unchanged (v2==v3) and must be absent from instructions";
}

TEST_F(PatcherTest, V2toV3_ModifiedFile_Logo_PresentInInstructions) {
  patcher::DiffEngine engine(fixDir("v2"), fixDir("v3"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  auto files = parseInstructions(outDir.path());
  EXPECT_TRUE(isInInstructions(files, "assets/logo.txt"));
}

// ─── Tests: v3 → v4 ─────────────────────────────────────────────────────────

TEST_F(PatcherTest, V3toV4_LargeDiff_Changelog_Succeeds) {
  patcher::DiffEngine engine(fixDir("v3"), fixDir("v4"), outDir.str());
  // changelog.txt is completely rewritten — exercises large Myers diff
  ASSERT_TRUE(engine.generatePatches());

  auto files = parseInstructions(outDir.path());
  EXPECT_TRUE(isInInstructions(files, "changelog.txt"));
}

TEST_F(PatcherTest, V3toV4_ModifiedConfig_PresentInInstructions) {
  patcher::DiffEngine engine(fixDir("v3"), fixDir("v4"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  auto files = parseInstructions(outDir.path());
  EXPECT_TRUE(isInInstructions(files, "app_config.cfg"));
}

// ─── Tests: identical directories ───────────────────────────────────────────

TEST_F(PatcherTest, IdenticalDirs_V1toV1_EmptyInstructions) {
  patcher::DiffEngine engine(fixDir("v1"), fixDir("v1"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  auto files = parseInstructions(outDir.path());
  // No file differs — instructions must be empty
  EXPECT_TRUE(files.empty())
      << "Patching v1→v1 must produce zero file entries";
}

TEST_F(PatcherTest, IdenticalDirs_V3toV3_EmptyInstructions) {
  patcher::DiffEngine engine(fixDir("v3"), fixDir("v3"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  auto files = parseInstructions(outDir.path());
  EXPECT_TRUE(files.empty());
}

// ─── Tests: file counts ──────────────────────────────────────────────────────

TEST_F(PatcherTest, V1toV2_InstructionCount_AtLeastFiveFiles) {
  patcher::DiffEngine engine(fixDir("v1"), fixDir("v2"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  auto files = parseInstructions(outDir.path());
  // v2 adds 2 new files and modifies at least 3 existing ones
  EXPECT_GE(files.size(), 5u);
}

TEST_F(PatcherTest, V2toV3_InstructionCount_IncludesDeletedAndModified) {
  patcher::DiffEngine engine(fixDir("v2"), fixDir("v3"), outDir.str());
  ASSERT_TRUE(engine.generatePatches());

  auto files = parseInstructions(outDir.path());
  // banner deleted, binary + logo + changelog modified = at least 4 entries
  EXPECT_GE(files.size(), 4u);
}



// ─── Entry point ────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new SuppressPatcherLogging());
  return RUN_ALL_TESTS();
}
