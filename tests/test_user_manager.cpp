/**
 * test_user_manager.cpp
 *
 * Tests for UserManager — the business-logic layer that owns user CRUD and
 * session (login/logout) state file management.
 *
 * Each fixture creates:
 *   - An in-memory SQLite DB for the user store.
 *   - A unique temporary file path for the state file (auto-cleaned up).
 *
 * Strict count verification:
 *   getAllUsers().size() is asserted after every mutation.
 */

#include "test_helpers.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "User.h"
#include "UserManager.h"

// ─── Helpers ────────────────────────────────────────────────────────────────

static User makeUser(const std::string& id, const std::string& name,
                     const std::string& password = "pass",
                     const std::string& pantryId = "") {
  User u;
  u.id = id;
  u.name = name;
  u.password = password;
  u.pantryId = pantryId;
  return u;
}

// ─── Fixture ────────────────────────────────────────────────────────────────

class UserManagerTest : public ::testing::Test {
 protected:
  TempPath stateFile;
  // ":memory:" for the DB; a unique temp path for the state file.
  UserManager mgr{":memory:", stateFile.path()};
};

// ─── User CRUD ───────────────────────────────────────────────────────────────

TEST_F(UserManagerTest, AddUser_NewUser_Succeeds) {
  User u = makeUser("u1", "Alice");
  ASSERT_TRUE(mgr.addUser(u));
  EXPECT_EQ(mgr.getAllUsers().size(), 1u);
}

TEST_F(UserManagerTest, AddUser_DuplicateName_ReturnsFalse) {
  User u1 = makeUser("u1", "Alice");
  User u2 = makeUser("u2", "Alice");  // same name, different id
  ASSERT_TRUE(mgr.addUser(u1));
  // UserManager checks existUser by name, so u2 is rejected
  EXPECT_FALSE(mgr.addUser(u2));
  EXPECT_EQ(mgr.getAllUsers().size(), 1u);
}

TEST_F(UserManagerTest, RemoveUser_DecreasesCount) {
  User u = makeUser("r1", "Remove me");
  ASSERT_TRUE(mgr.addUser(u));
  ASSERT_EQ(mgr.getAllUsers().size(), 1u);

  ASSERT_TRUE(mgr.removeUser(u));
  EXPECT_EQ(mgr.getAllUsers().size(), 0u);
}

TEST_F(UserManagerTest, UpdateUser_ChangesFields) {
  User u = makeUser("upd1", "Original", "oldpass", "old_pantry");
  ASSERT_TRUE(mgr.addUser(u));

  u.name = "Updated";
  u.password = "newpass";
  u.pantryId = "new_pantry";
  ASSERT_TRUE(mgr.updateUser(u));

  auto users = mgr.getAllUsers();
  ASSERT_EQ(users.size(), 1u);
  EXPECT_EQ(users[0].name, "Updated");
  EXPECT_EQ(users[0].password, "newpass");
  EXPECT_EQ(users[0].pantryId, "new_pantry");
}

TEST_F(UserManagerTest, GetAllUsers_ReturnsAllInserted) {
  for (int i = 0; i < 3; ++i) {
    ASSERT_TRUE(
        mgr.addUser(makeUser("id" + std::to_string(i), "User" + std::to_string(i))));
  }
  EXPECT_EQ(mgr.getAllUsers().size(), 3u);
}

// ─── Current user ────────────────────────────────────────────────────────────

TEST_F(UserManagerTest, SetCurrentUser_GetCurrentUser_Match) {
  User u = makeUser("cu1", "CurrentUser");
  mgr.setCurrentUser(u);

  auto cur = mgr.getCurrentUser();
  ASSERT_TRUE(cur.has_value());
  EXPECT_EQ(cur->id, "cu1");
  EXPECT_EQ(cur->name, "CurrentUser");
}

TEST_F(UserManagerTest, InitiallyNoCurrentUser) {
  EXPECT_FALSE(mgr.getCurrentUser().has_value());
}

// ─── Session / login-logout ──────────────────────────────────────────────────

TEST_F(UserManagerTest, SaveSession_CreatesFile) {
  User u = makeUser("s1", "SessionUser");
  ASSERT_TRUE(mgr.addUser(u));
  mgr.setCurrentUser(u);

  ASSERT_TRUE(mgr.saveSession());
  EXPECT_TRUE(std::filesystem::exists(stateFile.path()));
}

TEST_F(UserManagerTest, SaveSession_NoCurrentUser_ReturnsFalse) {
  EXPECT_FALSE(mgr.saveSession());
  EXPECT_FALSE(std::filesystem::exists(stateFile.path()));
}

TEST_F(UserManagerTest, LoadSession_RestoresCurrentUser) {
  User u = makeUser("ls1", "LoadMe");
  ASSERT_TRUE(mgr.addUser(u));
  mgr.setCurrentUser(u);
  ASSERT_TRUE(mgr.saveSession());

  // Clear in-memory state
  ASSERT_TRUE(mgr.clearSession());
  EXPECT_FALSE(mgr.getCurrentUser().has_value());

  // Now recreate manager pointing at the same state file + DB
  // We need to rebuild mgr since clearSession deletes the file.
  // Instead: manually write the state file back and call loadSession.
  mgr.setCurrentUser(u);
  ASSERT_TRUE(mgr.saveSession());  // re-write the file

  UserManager mgr2(":memory:", stateFile.path());
  // mgr2 has a fresh DB — we need to add the user so loadSession can find them
  ASSERT_TRUE(mgr2.addUser(u));
  ASSERT_TRUE(mgr2.loadSession());

  auto cur = mgr2.getCurrentUser();
  ASSERT_TRUE(cur.has_value());
  EXPECT_EQ(cur->id, "ls1");
  EXPECT_EQ(cur->name, "LoadMe");
}

TEST_F(UserManagerTest, ClearSession_DeletesFile) {
  User u = makeUser("cl1", "ClearMe");
  ASSERT_TRUE(mgr.addUser(u));
  mgr.setCurrentUser(u);
  ASSERT_TRUE(mgr.saveSession());
  ASSERT_TRUE(std::filesystem::exists(stateFile.path()));

  ASSERT_TRUE(mgr.clearSession());
  EXPECT_FALSE(std::filesystem::exists(stateFile.path()));
}

TEST_F(UserManagerTest, ClearSession_SetsCurrentUserToNullopt) {
  User u = makeUser("cl2", "ClearMe2");
  ASSERT_TRUE(mgr.addUser(u));
  mgr.setCurrentUser(u);
  ASSERT_TRUE(mgr.saveSession());

  ASSERT_TRUE(mgr.clearSession());
  EXPECT_FALSE(mgr.getCurrentUser().has_value());
}

TEST_F(UserManagerTest, LoadSession_NoFile_ReturnsFalse) {
  // State file does not exist at this point
  EXPECT_FALSE(mgr.loadSession());
  EXPECT_FALSE(mgr.getCurrentUser().has_value());
}

TEST_F(UserManagerTest, LoadSession_InvalidJson_ReturnsFalse) {
  // Write garbage to the state file
  {
    std::ofstream f(stateFile.path());
    f << "NOT_JSON{{{";
  }
  EXPECT_FALSE(mgr.loadSession());
}

TEST_F(UserManagerTest, LoadSession_UnknownId_ReturnsFalse) {
  // Write a valid JSON with an id that doesn't exist in the DB
  {
    std::ofstream f(stateFile.path());
    f << R"({"id":"no_such_user"})";
  }
  EXPECT_FALSE(mgr.loadSession());
}

TEST_F(UserManagerTest, RemoveActiveUser_ClearsSession) {
  User u = makeUser("ra1", "ActiveUser");
  ASSERT_TRUE(mgr.addUser(u));
  mgr.setCurrentUser(u);
  ASSERT_TRUE(mgr.saveSession());

  // Removing the currently active user should clear the session
  ASSERT_TRUE(mgr.removeUser(u));
  EXPECT_FALSE(mgr.getCurrentUser().has_value());
  EXPECT_FALSE(std::filesystem::exists(stateFile.path()));
}

TEST_F(UserManagerTest, UpdateActiveUser_ReflectedInCurrentUser) {
  User u = makeUser("ua1", "OldName");
  ASSERT_TRUE(mgr.addUser(u));
  mgr.setCurrentUser(u);

  u.name = "NewName";
  ASSERT_TRUE(mgr.updateUser(u));

  auto cur = mgr.getCurrentUser();
  ASSERT_TRUE(cur.has_value());
  EXPECT_EQ(cur->name, "NewName");
}

// ─── Multi-profile login/logout ──────────────────────────────────────────────

TEST_F(UserManagerTest, MultiProfile_LoginSwitchLogout) {
  User alice = makeUser("alice", "Alice");
  User bob = makeUser("bob", "Bob");
  ASSERT_TRUE(mgr.addUser(alice));
  ASSERT_TRUE(mgr.addUser(bob));

  // Login as Alice
  mgr.setCurrentUser(alice);
  ASSERT_TRUE(mgr.saveSession());
  EXPECT_EQ(mgr.getCurrentUser()->id, "alice");

  // Switch to Bob
  mgr.setCurrentUser(bob);
  ASSERT_TRUE(mgr.saveSession());
  EXPECT_EQ(mgr.getCurrentUser()->id, "bob");

  // Logout
  ASSERT_TRUE(mgr.clearSession());
  EXPECT_FALSE(mgr.getCurrentUser().has_value());
}

// ─── Entry point ────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new SuppressLogging());
  return RUN_ALL_TESTS();
}
