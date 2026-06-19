/**
 * test_file_manager.cpp
 *
 * Tests for FileManager — the base file I/O class used by StateFileManager
 * and KeyFileManager.
 *
 * Each test uses a unique TempPath so no files are left on disk.
 */

#include "test_helpers.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "FileManager.h"

// ─── Fixture ────────────────────────────────────────────────────────────────

class FileManagerTest : public ::testing::Test {
 protected:
  TempPath tempPath;
  FileManager fm{tempPath.path()};
};

// ─── Read / Write ────────────────────────────────────────────────────────────

TEST_F(FileManagerTest, WriteAndRead_RoundTrips) {
  const std::string data = "Hello, MarkIt!";
  ASSERT_TRUE(fm.writeFile(data));

  auto result = fm.readFile();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, data);
}

TEST_F(FileManagerTest, ReadFile_NonExistentFile_ReturnsNullopt) {
  auto result = fm.readFile();
  EXPECT_FALSE(result.has_value());
}

TEST_F(FileManagerTest, WriteEmptyString_ReadReturnsEmpty) {
  ASSERT_TRUE(fm.writeFile(""));
  auto result = fm.readFile();
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->empty());
}

TEST_F(FileManagerTest, WriteFile_Overwrite_NewContentVisible) {
  ASSERT_TRUE(fm.writeFile("first content"));
  ASSERT_TRUE(fm.writeFile("second content"));

  auto result = fm.readFile();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, "second content");
}

TEST_F(FileManagerTest, WriteLargeContent_RoundTrips) {
  std::string large(10'000, 'X');
  ASSERT_TRUE(fm.writeFile(large));

  auto result = fm.readFile();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, large);
}

TEST_F(FileManagerTest, WriteMultilineContent_RoundTrips) {
  const std::string multiline = "line1\nline2\nline3\n";
  ASSERT_TRUE(fm.writeFile(multiline));

  auto result = fm.readFile();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, multiline);
}

// ─── Delete ──────────────────────────────────────────────────────────────────

TEST_F(FileManagerTest, DeleteFile_ExistingFile_FileGone) {
  ASSERT_TRUE(fm.writeFile("data"));
  ASSERT_TRUE(std::filesystem::exists(tempPath.path()));

  ASSERT_TRUE(fm.deleteFile());
  EXPECT_FALSE(std::filesystem::exists(tempPath.path()));
}

TEST_F(FileManagerTest, DeleteFile_NonExistentFile_ReturnsTrue) {
  // Idempotent — must return true even if file doesn't exist
  EXPECT_TRUE(fm.deleteFile());
}

TEST_F(FileManagerTest, DeleteFile_ReadAfterDelete_ReturnsNullopt) {
  ASSERT_TRUE(fm.writeFile("some data"));
  ASSERT_TRUE(fm.deleteFile());
  EXPECT_FALSE(fm.readFile().has_value());
}

TEST_F(FileManagerTest, DeleteFile_Twice_BothReturnTrue) {
  ASSERT_TRUE(fm.writeFile("data"));
  ASSERT_TRUE(fm.deleteFile());
  // Second delete on a non-existent file is still true
  EXPECT_TRUE(fm.deleteFile());
}

// ─── Multiple independent instances ──────────────────────────────────────────

TEST_F(FileManagerTest, TwoInstances_DifferentPaths_Independent) {
  TempPath path2;
  FileManager fm2(path2.path());

  ASSERT_TRUE(fm.writeFile("content A"));
  ASSERT_TRUE(fm2.writeFile("content B"));

  EXPECT_EQ(*fm.readFile(), "content A");
  EXPECT_EQ(*fm2.readFile(), "content B");
}

// ─── Entry point ────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new SuppressLogging());
  return RUN_ALL_TESTS();
}
