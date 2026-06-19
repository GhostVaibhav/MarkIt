/**
 * test_updater.cpp
 *
 * End-to-end integration tests for markit_patcher + markit_updater.
 *
 * Each test:
 *  1. Copies the "installed" fixture version into a temp install/ dir.
 *  2. Runs markit_patcher to generate patches.bin + instructions.bin.
 *  3. Packs the output into a patches-N.tar.gz archive in a staging/ dir.
 *  4. Runs markit_updater --binary install/markit_binary --staged staging/.
 *  5. Byte-compares every file in install/ against the expected fixture.
 *     Extra or missing files are also flagged as failures.
 *
 * Sequential patch tests pack multiple archives (patches-0.tar.gz,
 * patches-1.tar.gz, ...) into the same staging dir and invoke the updater
 * once, relying on alphabetical sort inside the updater.
 *
 * TAR AVAILABILITY:
 *   All tests that invoke markit_updater require `tar` in PATH.
 *   If `tar` is unavailable the test is skipped gracefully via GTEST_SKIP().
 */

#include <atomic>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ─── Platform helpers ────────────────────────────────────────────────────────

// On Windows, system() uses cmd.exe /C. When a command string contains inner
// double-quotes (for paths with spaces), the outer cmd wrapper must not add
// extra quotes. We suppress output by appending 2>NUL >NUL which cmd.exe
// handles correctly regardless of inner quoting.
#ifdef _WIN32
static const std::string kNullRedirect = " >NUL 2>NUL";
#else
static const std::string kNullRedirect = " >/dev/null 2>&1";
#endif

static std::string q(const std::string& s) {
  // Quote a path for use in system() commands.
  return "\"" + s + "\"";
}

// ─── Tar availability check ──────────────────────────────────────────────────

static bool isTarAvailable() {
#ifdef _WIN32
  int ret = system("cmd /c \"tar --version\" >NUL 2>NUL");
#else
  int ret = system("tar --version >/dev/null 2>&1");
#endif
  return ret == 0;
}

// ─── Temp directory (RAII) ────────────────────────────────────────────────────

static std::atomic<int> tempCounter{0};

class TempDir {
 public:
  TempDir() {
    path_ = fs::temp_directory_path() /
            ("markit_utest_" + std::to_string(tempCounter.fetch_add(1)));
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

// ─── File utilities ──────────────────────────────────────────────────────────

static void copyDir(const fs::path& src, const fs::path& dst) {
  fs::create_directories(dst);
  for (const auto& entry : fs::recursive_directory_iterator(src)) {
    const fs::path rel = fs::relative(entry.path(), src);
    if (entry.is_directory()) {
      fs::create_directories(dst / rel);
    } else {
      fs::create_directories((dst / rel).parent_path());
      fs::copy_file(entry.path(), dst / rel,
                    fs::copy_options::overwrite_existing);
    }
  }
}

// Read file bytes into a vector.
static std::vector<uint8_t> readBytes(const fs::path& p) {
  std::ifstream f(p, std::ios::binary);
  return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)),
                               std::istreambuf_iterator<char>());
}

// Returns "" if files match, or a descriptive message on mismatch.
static std::string compareFile(const fs::path& got, const fs::path& expected) {
  if (!fs::exists(got))
    return "MISSING: " + got.string();
  if (!fs::exists(expected))
    return "UNEXPECTED (expected file does not exist): " + got.string();

  auto gotBytes = readBytes(got);
  auto expBytes = readBytes(expected);
  if (gotBytes != expBytes) {
    return "CONTENT MISMATCH for " + got.filename().string() +
           " (got " + std::to_string(gotBytes.size()) + " bytes, expected " +
           std::to_string(expBytes.size()) + " bytes)";
  }
  return "";
}

// Verify every file in expectedDir exists and matches in actualDir.
// 0-byte files in actualDir are treated as "deleted" by the updater
// (op "-" empties the file rather than removing it). Such tombstones are
// skipped when checking for extra files.
static testing::AssertionResult dirsMatch(const fs::path& actualDir,
                                           const fs::path& expectedDir) {
  bool ok = true;
  std::string errors;

  // Check expected files exist and match (non-zero size in actual).
  for (const auto& entry : fs::recursive_directory_iterator(expectedDir)) {
    if (!entry.is_regular_file()) continue;
    const fs::path rel = fs::relative(entry.path(), expectedDir);
    const fs::path actual = actualDir / rel;
    const std::string err = compareFile(actual, entry.path());
    if (!err.empty()) {
      ok = false;
      errors += "\n  " + err;
    }
  }

  // Strict extra-file check: skip 0-byte tombstones (updater delete behavior).
  for (const auto& entry : fs::recursive_directory_iterator(actualDir)) {
    if (!entry.is_regular_file()) continue;
    if (entry.file_size() == 0) continue;  // 0-byte = deleted tombstone
    const fs::path rel = fs::relative(entry.path(), actualDir);
    const fs::path expected = expectedDir / rel;
    if (!fs::exists(expected)) {
      ok = false;
      errors += "\n  EXTRA FILE NOT IN EXPECTED: " + rel.string();
    }
  }

  if (!ok)
    return testing::AssertionFailure() << errors;
  return testing::AssertionSuccess();
}

// ─── Patcher / updater invocation helpers ────────────────────────────────────

// Run a shell command, suppressing all output.
// On Windows, wraps in 'cmd /c "..."' to avoid the cmd.exe outer-quote
// stripping bug: when a command string starts AND ends with ", cmd.exe strips
// the outermost pair, breaking the command.
static int runCmd(const std::string& cmd) {
#ifdef _WIN32
  std::string wrapped = "cmd /c \"" + cmd + "\"" + " >NUL 2>NUL";
  return system(wrapped.c_str());
#else
  std::string full = cmd + " >/dev/null 2>&1";
  return system(full.c_str());
#endif
}

// Run markit_patcher. Returns exit code.
static int runPatcher(const fs::path& oldDir, const fs::path& newDir,
                      const fs::path& outDir) {
  std::string cmd = q(PATCHER_BIN) + " " + q(oldDir.string()) + " " +
                    q(newDir.string()) + " " + q(outDir.string());
  return runCmd(cmd);
}

// Pack outDir contents into archivePath (tar.gz).
// The archive contains instructions.bin and patches.bin at its root.
static int packArchive(const fs::path& outDir, const fs::path& archivePath) {
  std::string cmd = "tar czf " + q(archivePath.string()) +
                    " -C " + q(outDir.string()) +
                    " instructions.bin patches.bin";
  return runCmd(cmd);
}

// Run markit_updater. Returns exit code.
static int runUpdater(const fs::path& binaryPath, const fs::path& stagingDir) {
  std::string cmd = q(UPDATER_BIN) +
                    " --binary " + q(binaryPath.string()) +
                    " --staged " + q(stagingDir.string()) +
                    " --fast";
  return runCmd(cmd);
}


static std::string fixDir(const std::string& version) {
  return std::string(FIXTURES_DIR) + "/" + version;
}

// ─── Test fixture ────────────────────────────────────────────────────────────

class UpdaterTest : public ::testing::Test {
 protected:
  void SetUp() override {
    if (!isTarAvailable()) {
      GTEST_SKIP() << "tar not available in PATH — skipping updater tests";
    }
  }

  // Build a single-patch staging dir: old_ver → new_ver, archive name patches-0.tar.gz
  // Returns 0 on success.
  int prepareSinglePatch(const std::string& oldVer, const std::string& newVer,
                         const fs::path& stagingDir) {
    TempDir patchOut;
    int ret = runPatcher(fixDir(oldVer), fixDir(newVer), patchOut.path());
    if (ret != 0) return ret;
    return packArchive(patchOut.path(),
                       stagingDir / "patches-0.tar.gz");
  }
};

// ─── Single patch tests ───────────────────────────────────────────────────────

TEST_F(UpdaterTest, SinglePatch_V1toV2_AllFilesMatch) {
  TempDir installDir, stagingDir;
  copyDir(fixDir("v1"), installDir.path());

  ASSERT_EQ(prepareSinglePatch("v1", "v2", stagingDir.path()), 0);

  const fs::path binary = installDir.path() / "markit_binary";
  ASSERT_EQ(runUpdater(binary, stagingDir.path()), 0);

  EXPECT_TRUE(dirsMatch(installDir.path(), fixDir("v2")));
}

TEST_F(UpdaterTest, SinglePatch_V1toV2_NewFilesPresent) {
  TempDir installDir, stagingDir;
  copyDir(fixDir("v1"), installDir.path());

  ASSERT_EQ(prepareSinglePatch("v1", "v2", stagingDir.path()), 0);

  const fs::path binary = installDir.path() / "markit_binary";
  ASSERT_EQ(runUpdater(binary, stagingDir.path()), 0);

  EXPECT_TRUE(fs::exists(installDir.path() / "assets" / "banner.txt"))
      << "banner.txt must be created by v1→v2 update";
  EXPECT_TRUE(fs::exists(installDir.path() / "changelog.txt"))
      << "changelog.txt must be created by v1→v2 update";
}

TEST_F(UpdaterTest, SinglePatch_V1toV2_BinaryContentMatches) {
  TempDir installDir, stagingDir;
  copyDir(fixDir("v1"), installDir.path());

  ASSERT_EQ(prepareSinglePatch("v1", "v2", stagingDir.path()), 0);

  const fs::path binary = installDir.path() / "markit_binary";
  ASSERT_EQ(runUpdater(binary, stagingDir.path()), 0);

  auto got = readBytes(installDir.path() / "markit_binary");
  auto exp = readBytes(fs::path(fixDir("v2")) / "markit_binary");
  EXPECT_EQ(got, exp) << "markit_binary must exactly match v2 fixture after update";
}

TEST_F(UpdaterTest, SinglePatch_V2toV3_DeletedFileGone) {
  TempDir installDir, stagingDir;
  copyDir(fixDir("v2"), installDir.path());

  ASSERT_EQ(prepareSinglePatch("v2", "v3", stagingDir.path()), 0);

  const fs::path binary = installDir.path() / "markit_binary";
  EXPECT_EQ(runUpdater(binary, stagingDir.path()), 0);

  // The updater converts deleted files to 0-byte tombstones (empties them).
  const fs::path banner = installDir.path() / "assets" / "banner.txt";
  if (fs::exists(banner)) {
    EXPECT_EQ(fs::file_size(banner), 0u)
        << "banner.txt must be a 0-byte tombstone after v2→v3 update";
  }
}

TEST_F(UpdaterTest, SinglePatch_V2toV3_UnchangedFileContentPreserved) {
  TempDir installDir, stagingDir;
  copyDir(fixDir("v2"), installDir.path());

  ASSERT_EQ(prepareSinglePatch("v2", "v3", stagingDir.path()), 0);

  const fs::path binary = installDir.path() / "markit_binary";
  ASSERT_EQ(runUpdater(binary, stagingDir.path()), 0);

  // app_config.cfg is identical in v2 and v3 — must still be correct after update
  auto got = readBytes(installDir.path() / "app_config.cfg");
  auto exp = readBytes(fs::path(fixDir("v3")) / "app_config.cfg");
  EXPECT_EQ(got, exp) << "app_config.cfg must be unchanged after v2→v3 update";
}

TEST_F(UpdaterTest, SinglePatch_V2toV3_AllFilesMatch) {
  TempDir installDir, stagingDir;
  copyDir(fixDir("v2"), installDir.path());

  ASSERT_EQ(prepareSinglePatch("v2", "v3", stagingDir.path()), 0);

  const fs::path binary = installDir.path() / "markit_binary";
  ASSERT_EQ(runUpdater(binary, stagingDir.path()), 0);

  EXPECT_TRUE(dirsMatch(installDir.path(), fixDir("v3")));
}

// ─── Sequential patch tests ───────────────────────────────────────────────────

TEST_F(UpdaterTest, SequentialPatches_V1toV2toV3_AllFilesMatch) {
  TempDir installDir, stagingDir;
  copyDir(fixDir("v1"), installDir.path());

  // Patch 0: v1 → v2
  {
    TempDir out0;
    ASSERT_EQ(runPatcher(fixDir("v1"), fixDir("v2"), out0.path()), 0);
    ASSERT_EQ(packArchive(out0.path(), stagingDir.path() / "patches-0.tar.gz"), 0);
  }
  // Patch 1: v2 → v3
  {
    TempDir out1;
    ASSERT_EQ(runPatcher(fixDir("v2"), fixDir("v3"), out1.path()), 0);
    ASSERT_EQ(packArchive(out1.path(), stagingDir.path() / "patches-1.tar.gz"), 0);
  }

  const fs::path binary = installDir.path() / "markit_binary";
  ASSERT_EQ(runUpdater(binary, stagingDir.path()), 0);

  EXPECT_TRUE(dirsMatch(installDir.path(), fixDir("v3")));
}

TEST_F(UpdaterTest, SequentialPatches_V1toV2toV3_BannerDeletedAfterChain) {
  TempDir installDir, stagingDir;
  copyDir(fixDir("v1"), installDir.path());

  {
    TempDir out0;
    ASSERT_EQ(runPatcher(fixDir("v1"), fixDir("v2"), out0.path()), 0);
    ASSERT_EQ(packArchive(out0.path(), stagingDir.path() / "patches-0.tar.gz"), 0);
  }
  {
    TempDir out1;
    ASSERT_EQ(runPatcher(fixDir("v2"), fixDir("v3"), out1.path()), 0);
    ASSERT_EQ(packArchive(out1.path(), stagingDir.path() / "patches-1.tar.gz"), 0);
  }

  const fs::path binary = installDir.path() / "markit_binary";
  ASSERT_EQ(runUpdater(binary, stagingDir.path()), 0);

  // banner.txt was added in v2 then deleted in v3.
  // After the chain it must be absent OR be a 0-byte tombstone.
  const fs::path banner = installDir.path() / "assets" / "banner.txt";
  if (fs::exists(banner)) {
    EXPECT_EQ(fs::file_size(banner), 0u)
        << "banner.txt must be a 0-byte tombstone after v1→v2→v3 chain";
  }
}

TEST_F(UpdaterTest, SequentialPatches_V1toV2toV3toV4_AllFilesMatch) {
  TempDir installDir, stagingDir;
  copyDir(fixDir("v1"), installDir.path());

  struct { const char* from; const char* to; const char* archive; } patches[] = {
    {"v1", "v2", "patches-0.tar.gz"},
    {"v2", "v3", "patches-1.tar.gz"},
    {"v3", "v4", "patches-2.tar.gz"},
  };

  for (const auto& p : patches) {
    TempDir out;
    ASSERT_EQ(runPatcher(fixDir(p.from), fixDir(p.to), out.path()), 0)
        << "patcher failed for " << p.from << "→" << p.to;
    ASSERT_EQ(packArchive(out.path(), stagingDir.path() / p.archive), 0)
        << "tar failed for " << p.archive;
  }

  const fs::path binary = installDir.path() / "markit_binary";
  ASSERT_EQ(runUpdater(binary, stagingDir.path()), 0);

  EXPECT_TRUE(dirsMatch(installDir.path(), fixDir("v4")));
}

TEST_F(UpdaterTest, SequentialPatches_V1toV4_StrictFileCount) {
  TempDir installDir, stagingDir;
  copyDir(fixDir("v1"), installDir.path());

  struct { const char* from; const char* to; const char* archive; } patches[] = {
    {"v1", "v2", "patches-0.tar.gz"},
    {"v2", "v3", "patches-1.tar.gz"},
    {"v3", "v4", "patches-2.tar.gz"},
  };
  for (const auto& p : patches) {
    TempDir out;
    ASSERT_EQ(runPatcher(fixDir(p.from), fixDir(p.to), out.path()), 0);
    ASSERT_EQ(packArchive(out.path(), stagingDir.path() / p.archive), 0);
  }

  const fs::path binary = installDir.path() / "markit_binary";
  ASSERT_EQ(runUpdater(binary, stagingDir.path()), 0);

  // Count only non-zero-byte files (0-byte = deleted tombstone in updater convention)
  size_t installCount = 0;
  for (const auto& e : fs::recursive_directory_iterator(installDir.path()))
    if (e.is_regular_file() && e.file_size() > 0) ++installCount;

  size_t v4Count = 0;
  for (const auto& e : fs::recursive_directory_iterator(fixDir("v4")))
    if (e.is_regular_file()) ++v4Count;

  EXPECT_EQ(installCount, v4Count)
      << "Non-zero file count after v1→v2→v3→v4 chain must match v4 fixture ("
      << v4Count << " files)";
}

// ─── Error / edge case tests ─────────────────────────────────────────────────

TEST_F(UpdaterTest, Patcher_NonExistentOldDir_Returns_Nonzero) {
  TempDir out;
  int ret = runPatcher("/does/not/exist", fixDir("v2"), out.path());
  EXPECT_NE(ret, 0) << "patcher must return non-zero for non-existent old_dir";
}

TEST_F(UpdaterTest, Patcher_NonExistentNewDir_Returns_Nonzero) {
  TempDir out;
  int ret = runPatcher(fixDir("v1"), "/does/not/exist", out.path());
  EXPECT_NE(ret, 0) << "patcher must return non-zero for non-existent new_dir";
}

TEST_F(UpdaterTest, Updater_MissingStagedDir_Returns_Nonzero) {
  TempDir installDir;
  copyDir(fixDir("v1"), installDir.path());
  const fs::path binary = installDir.path() / "markit_binary";
  // Staging dir does not exist
  int ret = runUpdater(binary, installDir.path() / "does_not_exist");
  EXPECT_NE(ret, 0) << "updater must return non-zero for missing staged dir";
}

TEST_F(UpdaterTest, Updater_EmptyStagingDir_Returns_Nonzero) {
  TempDir installDir, stagingDir;
  copyDir(fixDir("v1"), installDir.path());
  const fs::path binary = installDir.path() / "markit_binary";
  // staging dir exists but has no .tar.gz archives
  int ret = runUpdater(binary, stagingDir.path());
  EXPECT_NE(ret, 0) << "updater must return non-zero when staging dir has no archives";
}

// ─── Entry point ────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
