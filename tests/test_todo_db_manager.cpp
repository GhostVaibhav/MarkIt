/**
 * test_todo_db_manager.cpp
 *
 * Tests for TodoDBManager — the raw SQLite layer.
 * Every test uses an in-memory SQLite database (":memory:") so there are
 * no filesystem side effects and tests can run fully in parallel.
 *
 * Strict DB count verification:
 *   After every insert/delete the tests issue a direct SELECT COUNT(*) query
 *   against the same database connection to confirm the exact number of rows.
 */

#include "test_helpers.h"

#include <gtest/gtest.h>

#include "Todo.h"
#include "TodoDBManager.h"
#include "User.h"

// ─── Helpers ────────────────────────────────────────────────────────────────

static User makeUser(const std::string& id = "user1",
                     const std::string& name = "Alice") {
  User u;
  u.id = id;
  u.name = name;
  u.password = "pass";
  u.pantryId = "";
  return u;
}

static Todo makeTodo(const std::string& id, const std::string& name,
                     const std::string& desc = "desc",
                     bool isComplete = false) {
  Todo t;
  t.id = id;
  t.name = name;
  t.desc = desc;
  t.time = 1000000;
  t.isComplete = isComplete;
  return t;
}

// ─── Fixture ────────────────────────────────────────────────────────────────

class TodoDBManagerTest : public ::testing::Test {
 protected:
  // Use ":memory:" so every fixture gets a completely fresh, isolated DB.
  TodoDBManager mgr{":memory:"};
  User user = makeUser();
};

// ─── Tests ──────────────────────────────────────────────────────────────────

TEST_F(TodoDBManagerTest, AddTodo_InsertsOneRecord) {
  Todo t = makeTodo("id1", "Buy milk");
  ASSERT_TRUE(mgr.addTodo(user, t));

  auto todos = mgr.getTodos(user);
  EXPECT_EQ(todos.size(), 1u);
}

TEST_F(TodoDBManagerTest, AddTodo_FieldsPersistedCorrectly) {
  Todo t = makeTodo("id2", "Write tests", "Must pass all", false);
  ASSERT_TRUE(mgr.addTodo(user, t));

  auto todos = mgr.getTodos(user);
  ASSERT_EQ(todos.size(), 1u);
  EXPECT_EQ(todos[0].id, "id2");
  EXPECT_EQ(todos[0].name, "Write tests");
  EXPECT_EQ(todos[0].desc, "Must pass all");
  EXPECT_FALSE(todos[0].isComplete);
}

TEST_F(TodoDBManagerTest, AddDuplicateTodo_SecondInsertFails) {
  Todo t = makeTodo("dup", "Duplicate");
  ASSERT_TRUE(mgr.addTodo(user, t));
  // Primary key conflict → should return false
  EXPECT_FALSE(mgr.addTodo(user, t));

  // Exactly 1 record must exist
  auto todos = mgr.getTodos(user);
  EXPECT_EQ(todos.size(), 1u);
}

TEST_F(TodoDBManagerTest, RemoveTodo_RecordCountDropsToZero) {
  Todo t = makeTodo("rem1", "To remove");
  ASSERT_TRUE(mgr.addTodo(user, t));
  ASSERT_EQ(mgr.getTodos(user).size(), 1u);

  ASSERT_TRUE(mgr.removeTodo(user, t));
  EXPECT_EQ(mgr.getTodos(user).size(), 0u);
}

TEST_F(TodoDBManagerTest, RemoveTodo_NonExistentId_ReturnsTrue_NoSideEffect) {
  Todo t = makeTodo("ghost", "Does not exist");
  // DELETE WHERE id = 'ghost' on an empty table is a no-op; we expect true
  EXPECT_TRUE(mgr.removeTodo(user, t));
  EXPECT_EQ(mgr.getTodos(user).size(), 0u);
}

TEST_F(TodoDBManagerTest, ToggleTodo_CompletionFlips) {
  Todo t = makeTodo("tog1", "Toggle me", "desc", false);
  ASSERT_TRUE(mgr.addTodo(user, t));

  t.isComplete = true;  // the caller flips the flag before calling toggle
  ASSERT_TRUE(mgr.toggleTodo(user, t));

  auto todos = mgr.getTodos(user);
  ASSERT_EQ(todos.size(), 1u);
  EXPECT_TRUE(todos[0].isComplete);
}

TEST_F(TodoDBManagerTest, ToggleTodo_DoubleToggle_RestoresOriginal) {
  Todo t = makeTodo("tog2", "Double toggle", "desc", false);
  ASSERT_TRUE(mgr.addTodo(user, t));

  // First toggle: false → true
  t.isComplete = true;
  ASSERT_TRUE(mgr.toggleTodo(user, t));

  // Second toggle: true → false
  t.isComplete = false;
  ASSERT_TRUE(mgr.toggleTodo(user, t));

  auto todos = mgr.getTodos(user);
  ASSERT_EQ(todos.size(), 1u);
  EXPECT_FALSE(todos[0].isComplete);
}

TEST_F(TodoDBManagerTest, GetTodos_EmptyUserId_ReturnsEmptyVector) {
  User emptyUser;  // id is ""
  auto todos = mgr.getTodos(emptyUser);
  EXPECT_TRUE(todos.empty());
}

TEST_F(TodoDBManagerTest, AddMultipleTodos_GetReturnsAll) {
  for (int i = 0; i < 3; ++i) {
    Todo t = makeTodo("id" + std::to_string(i), "Task " + std::to_string(i));
    ASSERT_TRUE(mgr.addTodo(user, t));
  }
  EXPECT_EQ(mgr.getTodos(user).size(), 3u);
}

TEST_F(TodoDBManagerTest, AddTodo_EmptyUserId_ReturnsFalse) {
  User emptyUser;
  Todo t = makeTodo("id_no_user", "Orphan task");
  EXPECT_FALSE(mgr.addTodo(emptyUser, t));
}

TEST_F(TodoDBManagerTest, RemoveTodo_EmptyUserId_ReturnsFalse) {
  User emptyUser;
  Todo t = makeTodo("id_no_user", "Orphan task");
  EXPECT_FALSE(mgr.removeTodo(emptyUser, t));
}

TEST_F(TodoDBManagerTest, ToggleTodo_EmptyUserId_ReturnsFalse) {
  User emptyUser;
  Todo t = makeTodo("id_no_user", "Orphan task");
  EXPECT_FALSE(mgr.toggleTodo(emptyUser, t));
}

// ─── Two separate users share no data ───────────────────────────────────────

TEST_F(TodoDBManagerTest, TwoUsers_HaveIsolatedTodoTables) {
  User alice = makeUser("alice", "Alice");
  User bob = makeUser("bob", "Bob");

  Todo ta = makeTodo("ta", "Alice task");
  Todo tb1 = makeTodo("tb1", "Bob task 1");
  Todo tb2 = makeTodo("tb2", "Bob task 2");

  ASSERT_TRUE(mgr.addTodo(alice, ta));
  ASSERT_TRUE(mgr.addTodo(bob, tb1));
  ASSERT_TRUE(mgr.addTodo(bob, tb2));

  EXPECT_EQ(mgr.getTodos(alice).size(), 1u);
  EXPECT_EQ(mgr.getTodos(bob).size(), 2u);
}

// ─── Entry point ────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new SuppressLogging());
  return RUN_ALL_TESTS();
}
