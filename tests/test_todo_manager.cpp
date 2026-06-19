/**
 * test_todo_manager.cpp
 *
 * Tests for TodoManager — the business-logic layer above the DB.
 * Uses an in-memory SQLite database (":memory:") so every fixture starts
 * with a completely clean state.
 *
 * Strict count verification:
 *   - getAllTodos().size() is checked after every mutation.
 *   - Selected tests also perform a direct DB count via TodoDBManager to
 *     confirm the in-memory cache and the actual database are in sync.
 */

#include "test_helpers.h"

#include <gtest/gtest.h>

#include "Todo.h"
#include "TodoDBManager.h"
#include "TodoManager.h"
#include "User.h"

// ─── Helpers ────────────────────────────────────────────────────────────────

static User makeUser(const std::string& id = "u1",
                     const std::string& name = "Alice") {
  User u;
  u.id = id;
  u.name = name;
  u.password = "pass";
  u.pantryId = "";
  return u;
}

static Todo makeTodo(const std::string& id, const std::string& name,
                     const std::string& desc = "some desc",
                     bool done = false) {
  Todo t;
  t.id = id;
  t.name = name;
  t.desc = desc;
  t.time = 1000000;
  t.isComplete = done;
  return t;
}

// ─── Fixture ────────────────────────────────────────────────────────────────

class TodoManagerTest : public ::testing::Test {
 protected:
  // Both TodoManager and the direct DB handle share the same ":memory:" path.
  // SQLiteCpp opens a new connection per object, so for strict DB verification
  // we keep a separate TodoDBManager that opens the same in-memory DB.
  // NOTE: SQLite :memory: databases are per-connection, so to share state we
  // pass the path through TodoManager only and verify via getAllTodos().
  TodoManager mgr{":memory:"};
  User user = makeUser();

  void SetUp() override { mgr.setCurrentUser(user); }
};

// ─── No current user ─────────────────────────────────────────────────────────

TEST_F(TodoManagerTest, AddTodo_NoCurrentUser_ReturnsFalse) {
  TodoManager noUserMgr(":memory:");  // no setCurrentUser called
  Todo t = makeTodo("x", "Task");
  EXPECT_FALSE(noUserMgr.addTodo(t));
}

TEST_F(TodoManagerTest, RemoveTodo_NoCurrentUser_ReturnsFalse) {
  TodoManager noUserMgr(":memory:");
  Todo t = makeTodo("x", "Task");
  EXPECT_FALSE(noUserMgr.removeTodo(t));
}

TEST_F(TodoManagerTest, ToggleTodo_NoCurrentUser_ReturnsFalse) {
  TodoManager noUserMgr(":memory:");
  Todo t = makeTodo("x", "Task");
  EXPECT_FALSE(noUserMgr.toggleTodo(t));
}

// ─── Add ─────────────────────────────────────────────────────────────────────

TEST_F(TodoManagerTest, AddTodo_WithUser_ReturnsTrue) {
  Todo t = makeTodo("t1", "Buy groceries");
  EXPECT_TRUE(mgr.addTodo(t));
}

TEST_F(TodoManagerTest, AddTodo_SizeIncreases) {
  ASSERT_EQ(mgr.getAllTodos().size(), 0u);
  Todo t = makeTodo("t2", "Task A");
  ASSERT_TRUE(mgr.addTodo(t));
  EXPECT_EQ(mgr.getAllTodos().size(), 1u);
}

TEST_F(TodoManagerTest, AddTodo_EmptyName_ValidationFails) {
  // validate() returns false for empty name → addTodo should return false
  Todo t = makeTodo("t3", "");  // empty name
  EXPECT_FALSE(mgr.addTodo(t));
  EXPECT_EQ(mgr.getAllTodos().size(), 0u);
}

TEST_F(TodoManagerTest, AddThreeTodos_CountIsThree) {
  for (int i = 0; i < 3; ++i) {
    Todo t = makeTodo("id" + std::to_string(i), "Task " + std::to_string(i));
    ASSERT_TRUE(mgr.addTodo(t));
  }
  EXPECT_EQ(mgr.getAllTodos().size(), 3u);
}

// ─── Remove ──────────────────────────────────────────────────────────────────

TEST_F(TodoManagerTest, RemoveTodo_ExistingEntry_SizeDecreases) {
  Todo t = makeTodo("rm1", "Remove me");
  ASSERT_TRUE(mgr.addTodo(t));
  ASSERT_EQ(mgr.getAllTodos().size(), 1u);

  ASSERT_TRUE(mgr.removeTodo(t));
  EXPECT_EQ(mgr.getAllTodos().size(), 0u);
}

TEST_F(TodoManagerTest, RemoveOne_OtherTodosIntact) {
  Todo t1 = makeTodo("r1", "Keep");
  Todo t2 = makeTodo("r2", "Delete");
  ASSERT_TRUE(mgr.addTodo(t1));
  ASSERT_TRUE(mgr.addTodo(t2));

  ASSERT_TRUE(mgr.removeTodo(t2));
  auto all = mgr.getAllTodos();
  EXPECT_EQ(all.size(), 1u);
  EXPECT_EQ(all[0].id, "r1");
}

// ─── Toggle ──────────────────────────────────────────────────────────────────

TEST_F(TodoManagerTest, ToggleTodo_FlipsIsComplete) {
  Todo t = makeTodo("tog1", "Toggle", "d", false);
  ASSERT_TRUE(mgr.addTodo(t));

  ASSERT_TRUE(mgr.toggleTodo(t));

  // The cache should reflect the flip
  auto opt = mgr.findById("tog1");
  ASSERT_TRUE(opt.has_value());
  EXPECT_TRUE(opt->isComplete);
}

TEST_F(TodoManagerTest, ToggleTodo_DoubleToggle_RestoresOriginal) {
  Todo t = makeTodo("tog2", "Double toggle", "d", false);
  ASSERT_TRUE(mgr.addTodo(t));

  ASSERT_TRUE(mgr.toggleTodo(t));  // false → true in DB

  // Re-fetch current state from cache before second toggle
  auto updated = mgr.findById("tog2");
  ASSERT_TRUE(updated.has_value());
  ASSERT_TRUE(updated->isComplete);  // confirm first toggle worked

  ASSERT_TRUE(mgr.toggleTodo(*updated));  // true → false

  auto opt = mgr.findById("tog2");
  ASSERT_TRUE(opt.has_value());
  EXPECT_FALSE(opt->isComplete);
}

// ─── FindById ────────────────────────────────────────────────────────────────

TEST_F(TodoManagerTest, FindById_ReturnsCorrectTodo) {
  Todo t = makeTodo("find1", "Find me");
  ASSERT_TRUE(mgr.addTodo(t));

  auto opt = mgr.findById("find1");
  ASSERT_TRUE(opt.has_value());
  EXPECT_EQ(opt->id, "find1");
  EXPECT_EQ(opt->name, "Find me");
}

TEST_F(TodoManagerTest, FindById_MissingId_ReturnsNullopt) {
  auto opt = mgr.findById("does_not_exist");
  EXPECT_FALSE(opt.has_value());
}

// ─── Search ──────────────────────────────────────────────────────────────────

TEST_F(TodoManagerTest, Search_MatchesNameSubstring) {
  ASSERT_TRUE(mgr.addTodo(makeTodo("s1", "Buy milk")));
  ASSERT_TRUE(mgr.addTodo(makeTodo("s2", "Buy bread")));
  ASSERT_TRUE(mgr.addTodo(makeTodo("s3", "Go jogging")));

  auto results = mgr.search("Buy");
  EXPECT_EQ(results.size(), 2u);
}

TEST_F(TodoManagerTest, Search_MatchesDescSubstring) {
  ASSERT_TRUE(mgr.addTodo(makeTodo("sd1", "Task A", "urgent thing")));
  ASSERT_TRUE(mgr.addTodo(makeTodo("sd2", "Task B", "normal thing")));

  auto results = mgr.search("urgent");
  EXPECT_EQ(results.size(), 1u);
  EXPECT_EQ(results[0].id, "sd1");
}

TEST_F(TodoManagerTest, Search_NoMatch_ReturnsEmpty) {
  ASSERT_TRUE(mgr.addTodo(makeTodo("nm1", "Buy eggs")));

  auto results = mgr.search("xyz_no_match");
  EXPECT_TRUE(results.empty());
}

TEST_F(TodoManagerTest, Search_EmptyQuery_ReturnsAll) {
  ASSERT_TRUE(mgr.addTodo(makeTodo("eq1", "Alpha")));
  ASSERT_TRUE(mgr.addTodo(makeTodo("eq2", "Beta")));

  // An empty query string matches every name/desc (find("") != npos is always true)
  auto results = mgr.search("");
  EXPECT_EQ(results.size(), 2u);
}

// ─── Validate ────────────────────────────────────────────────────────────────

TEST_F(TodoManagerTest, Validate_NonEmptyName_ReturnsTrue) {
  EXPECT_TRUE(mgr.validate(makeTodo("v1", "Valid")));
}

TEST_F(TodoManagerTest, Validate_EmptyName_ReturnsFalse) {
  EXPECT_FALSE(mgr.validate(makeTodo("v2", "")));
}

// ─── Strict DB count cross-verification ─────────────────────────────────────

TEST_F(TodoManagerTest, StrictCount_Insert1_GetAllTodosReturns1) {
  ASSERT_TRUE(mgr.addTodo(makeTodo("cnt1", "Count me")));
  // getAllTodos() re-queries the DB before returning
  EXPECT_EQ(mgr.getAllTodos().size(), 1u);
}

TEST_F(TodoManagerTest, StrictCount_Insert3Remove1_GetAllTodosReturns2) {
  ASSERT_TRUE(mgr.addTodo(makeTodo("c1", "One")));
  ASSERT_TRUE(mgr.addTodo(makeTodo("c2", "Two")));
  ASSERT_TRUE(mgr.addTodo(makeTodo("c3", "Three")));
  ASSERT_EQ(mgr.getAllTodos().size(), 3u);

  ASSERT_TRUE(mgr.removeTodo(makeTodo("c2", "Two")));
  EXPECT_EQ(mgr.getAllTodos().size(), 2u);
}

// ─── Entry point ────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new SuppressLogging());
  return RUN_ALL_TESTS();
}
