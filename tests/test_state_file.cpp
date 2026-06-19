/**
 * test_state_file.cpp
 *
 * Tests for StateFileManager — the login/logout session state file.
 * Covers read/write/delete round-trips, overwrite semantics, idempotent
 * delete, and multi-profile session switching.
 *
 * Each test uses a unique TempPath (RAII) so no files are left behind.
 */

#include "test_helpers.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "StateFileManager.h"

// ─── Fixture ────────────────────────────────────────────────────────────────

class StateFileTest : public ::testing::Test {
 protected:
  TempPath tempPath;
  StateFileManager sfm{tempPath.path()};
};

// ─── Basic read / write ──────────────────────────────────────────────────────

TEST_F(StateFileTest, WriteAndRead_RoundTrips) {
  const std::string content = R"({"id":"user42"})";
  ASSERT_TRUE(sfm.writeFile(content));

  auto read = sfm.readFile();
  ASSERT_TRUE(read.has_value());
  EXPECT_EQ(*read, content);
}

TEST_F(StateFileTest, ReadFile_NonExistent_ReturnsNullopt) {
  // File has not been written yet
  auto result = sfm.readFile();
  EXPECT_FALSE(result.has_value());
}

TEST_F(StateFileTest, WriteEmptyString_ReadReturnsEmpty) {
  ASSERT_TRUE(sfm.writeFile(""));
  auto read = sfm.readFile();
  ASSERT_TRUE(read.has_value());
  EXPECT_TRUE(read->empty());
}

TEST_F(StateFileTest, WriteFile_Overwrite_NewContentVisible) {
  ASSERT_TRUE(sfm.writeFile(R"({"id":"old_user"})"));
  ASSERT_TRUE(sfm.writeFile(R"({"id":"new_user"})"));

  auto read = sfm.readFile();
  ASSERT_TRUE(read.has_value());
  EXPECT_EQ(*read, R"({"id":"new_user"})");
}

// ─── Delete ──────────────────────────────────────────────────────────────────

TEST_F(StateFileTest, DeleteFile_ExistingFile_FileGone) {
  ASSERT_TRUE(sfm.writeFile(R"({"id":"gone"})"));
  ASSERT_TRUE(std::filesystem::exists(tempPath.path()));

  ASSERT_TRUE(sfm.deleteFile());
  EXPECT_FALSE(std::filesystem::exists(tempPath.path()));
}

TEST_F(StateFileTest, DeleteFile_NonExistent_ReturnsTrue) {
  // Idempotent delete — file does not exist, must still return true
  EXPECT_TRUE(sfm.deleteFile());
}

TEST_F(StateFileTest, DeleteFile_ReadAfterDelete_ReturnsNullopt) {
  ASSERT_TRUE(sfm.writeFile(R"({"id":"u1"})"));
  ASSERT_TRUE(sfm.deleteFile());

  EXPECT_FALSE(sfm.readFile().has_value());
}

// ─── Multi-profile login / logout simulation ─────────────────────────────────

TEST_F(StateFileTest, MultiProfile_WriteAlice_ReadAlice) {
  const std::string aliceJson = R"({"id":"alice"})";
  ASSERT_TRUE(sfm.writeFile(aliceJson));

  auto read = sfm.readFile();
  ASSERT_TRUE(read.has_value());
  EXPECT_EQ(*read, aliceJson);
}

TEST_F(StateFileTest, MultiProfile_SwitchFromAliceToBob) {
  // Login as Alice
  ASSERT_TRUE(sfm.writeFile(R"({"id":"alice"})"));
  auto alice = sfm.readFile();
  ASSERT_TRUE(alice.has_value());
  EXPECT_EQ(*alice, R"({"id":"alice"})");

  // Switch to Bob (overwrite)
  ASSERT_TRUE(sfm.writeFile(R"({"id":"bob"})"));
  auto bob = sfm.readFile();
  ASSERT_TRUE(bob.has_value());
  EXPECT_EQ(*bob, R"({"id":"bob"})");
}

TEST_F(StateFileTest, MultiProfile_LogoutAfterSwitch) {
  // Login as Alice, switch to Bob, then logout
  ASSERT_TRUE(sfm.writeFile(R"({"id":"alice"})"));
  ASSERT_TRUE(sfm.writeFile(R"({"id":"bob"})"));
  ASSERT_TRUE(sfm.deleteFile());

  EXPECT_FALSE(sfm.readFile().has_value());
  EXPECT_FALSE(std::filesystem::exists(tempPath.path()));
}

TEST_F(StateFileTest, MultiProfile_TwoDifferentFiles_Independent) {
  TempPath path2;
  StateFileManager sfm2(path2.path());

  ASSERT_TRUE(sfm.writeFile(R"({"id":"alice"})"));
  ASSERT_TRUE(sfm2.writeFile(R"({"id":"bob"})"));

  // Each manager reads its own file
  EXPECT_EQ(*sfm.readFile(), R"({"id":"alice"})");
  EXPECT_EQ(*sfm2.readFile(), R"({"id":"bob"})");
}

// ─── Entry point ────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new SuppressLogging());
  return RUN_ALL_TESTS();
}
