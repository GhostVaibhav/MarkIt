/**
 * test_user_db_manager.cpp
 *
 * Tests for UserDBManager — the raw SQLite user layer.
 * All tests use an in-memory database (":memory:") for full isolation.
 *
 * Strict count verification:
 *   getUsers().size() is checked after every mutation to confirm the exact
 *   number of rows stored in the database.
 */

#include "test_helpers.h"

#include <gtest/gtest.h>

#include "User.h"
#include "UserDBManager.h"

// ─── Helpers ────────────────────────────────────────────────────────────────

static User makeUser(const std::string& id, const std::string& name,
                     const std::string& password = "secret",
                     const std::string& pantryId = "") {
  User u;
  u.id = id;
  u.name = name;
  u.password = password;
  u.pantryId = pantryId;
  return u;
}

// ─── Fixture ────────────────────────────────────────────────────────────────

class UserDBManagerTest : public ::testing::Test {
 protected:
  UserDBManager mgr{":memory:"};
};

// ─── Tests ──────────────────────────────────────────────────────────────────

TEST_F(UserDBManagerTest, AddUser_InsertsOneRecord) {
  User u = makeUser("u1", "Alice");
  ASSERT_TRUE(mgr.addUser(u));

  EXPECT_EQ(mgr.getUsers().size(), 1u);
}

TEST_F(UserDBManagerTest, AddUser_FieldsPersistedCorrectly) {
  User u = makeUser("u2", "Bob", "bobpass", "pantry123");
  ASSERT_TRUE(mgr.addUser(u));

  auto users = mgr.getUsers();
  ASSERT_EQ(users.size(), 1u);
  EXPECT_EQ(users[0].id, "u2");
  EXPECT_EQ(users[0].name, "Bob");
  EXPECT_EQ(users[0].password, "bobpass");
  EXPECT_EQ(users[0].pantryId, "pantry123");
}

TEST_F(UserDBManagerTest, AddDuplicateUser_PrimaryKeyViolation_ReturnsFalse) {
  User u = makeUser("dup", "Duplicate");
  ASSERT_TRUE(mgr.addUser(u));
  // Same primary key → SQLite constraint violation
  EXPECT_FALSE(mgr.addUser(u));

  // Strictly one row must exist
  EXPECT_EQ(mgr.getUsers().size(), 1u);
}

TEST_F(UserDBManagerTest, RemoveUser_RecordCountDropsToZero) {
  User u = makeUser("r1", "To Remove");
  ASSERT_TRUE(mgr.addUser(u));
  ASSERT_EQ(mgr.getUsers().size(), 1u);

  ASSERT_TRUE(mgr.removeUser(u));
  EXPECT_EQ(mgr.getUsers().size(), 0u);
}

TEST_F(UserDBManagerTest, RemoveUser_NonExistent_ReturnsTrueNoSideEffect) {
  User u = makeUser("ghost", "Ghost");
  // DELETE WHERE id='ghost' on empty table → no-op, returns true
  EXPECT_TRUE(mgr.removeUser(u));
  EXPECT_EQ(mgr.getUsers().size(), 0u);
}

TEST_F(UserDBManagerTest, ExistUser_AfterAdd_ReturnsTrue) {
  User u = makeUser("e1", "ExistUser");
  ASSERT_TRUE(mgr.addUser(u));
  EXPECT_TRUE(mgr.existUser(u));
}

TEST_F(UserDBManagerTest, ExistUser_BeforeAdd_ReturnsFalse) {
  User u = makeUser("ne1", "NoExistUser");
  EXPECT_FALSE(mgr.existUser(u));
}

TEST_F(UserDBManagerTest, UpdateUser_ChangesFields) {
  User u = makeUser("upd1", "Original", "oldpass", "oldpantry");
  ASSERT_TRUE(mgr.addUser(u));

  u.name = "Updated";
  u.password = "newpass";
  u.pantryId = "newpantry";
  ASSERT_TRUE(mgr.updateUser(u));

  auto users = mgr.getUsers();
  ASSERT_EQ(users.size(), 1u);
  EXPECT_EQ(users[0].name, "Updated");
  EXPECT_EQ(users[0].password, "newpass");
  EXPECT_EQ(users[0].pantryId, "newpantry");
}

TEST_F(UserDBManagerTest, GetUsers_ReturnsAllInserted) {
  for (int i = 0; i < 3; ++i) {
    User u = makeUser("uid" + std::to_string(i), "User" + std::to_string(i));
    ASSERT_TRUE(mgr.addUser(u));
  }
  EXPECT_EQ(mgr.getUsers().size(), 3u);
}

TEST_F(UserDBManagerTest, RemoveOneOfMany_CountDecrements) {
  User u1 = makeUser("a", "Alpha");
  User u2 = makeUser("b", "Beta");
  User u3 = makeUser("c", "Gamma");
  ASSERT_TRUE(mgr.addUser(u1));
  ASSERT_TRUE(mgr.addUser(u2));
  ASSERT_TRUE(mgr.addUser(u3));
  ASSERT_EQ(mgr.getUsers().size(), 3u);

  ASSERT_TRUE(mgr.removeUser(u2));
  EXPECT_EQ(mgr.getUsers().size(), 2u);
}

// ─── Entry point ────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new SuppressLogging());
  return RUN_ALL_TESTS();
}
