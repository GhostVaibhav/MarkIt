/**
 * test_commands.cpp
 *
 * Tests for the Command pattern:
 *   AddTodoCommand, DeleteTodoCommand, ToggleTodoCommand — and their undo().
 *
 * All commands are exercised through a TodoManager backed by an in-memory DB.
 */

#include "test_helpers.h"

#include <gtest/gtest.h>

#include "AddTodoCommand.h"
#include "DeleteTodoCommand.h"
#include "Todo.h"
#include "TodoManager.h"
#include "ToggleTodoCommand.h"
#include "User.h"

// ─── Helpers ────────────────────────────────────────────────────────────────

static User makeUser() {
  User u;
  u.id = "cmd_user";
  u.name = "CmdUser";
  u.password = "pass";
  u.pantryId = "";
  return u;
}

static Todo makeTodo(const std::string& id, const std::string& name,
                     bool done = false) {
  Todo t;
  t.id = id;
  t.name = name;
  t.desc = "cmd test desc";
  t.time = 1000000;
  t.isComplete = done;
  return t;
}

// ─── Fixture ────────────────────────────────────────────────────────────────

class CommandTest : public ::testing::Test {
 protected:
  TodoManager mgr{":memory:"};
  User user = makeUser();

  void SetUp() override { mgr.setCurrentUser(user); }
};

// ─── AddTodoCommand ──────────────────────────────────────────────────────────

TEST_F(CommandTest, AddTodoCommand_Execute_TodoExists) {
  Todo t = makeTodo("add1", "AddMe");
  AddTodoCommand cmd(mgr, t);
  cmd.execute();

  EXPECT_EQ(mgr.getAllTodos().size(), 1u);
  auto opt = mgr.findById("add1");
  ASSERT_TRUE(opt.has_value());
  EXPECT_EQ(opt->name, "AddMe");
}

TEST_F(CommandTest, AddTodoCommand_Undo_TodoRemoved) {
  Todo t = makeTodo("add2", "UndoAdd");
  AddTodoCommand cmd(mgr, t);
  cmd.execute();
  ASSERT_EQ(mgr.getAllTodos().size(), 1u);

  cmd.undo();
  EXPECT_EQ(mgr.getAllTodos().size(), 0u);
  EXPECT_FALSE(mgr.findById("add2").has_value());
}

TEST_F(CommandTest, AddTodoCommand_ExecuteTwice_CountIsTwo) {
  Todo t1 = makeTodo("a1", "First");
  Todo t2 = makeTodo("a2", "Second");
  AddTodoCommand cmd1(mgr, t1);
  AddTodoCommand cmd2(mgr, t2);
  cmd1.execute();
  cmd2.execute();
  EXPECT_EQ(mgr.getAllTodos().size(), 2u);
}

// ─── DeleteTodoCommand ───────────────────────────────────────────────────────

TEST_F(CommandTest, DeleteTodoCommand_Execute_TodoRemoved) {
  Todo t = makeTodo("del1", "DeleteMe");
  ASSERT_TRUE(mgr.addTodo(t));
  ASSERT_EQ(mgr.getAllTodos().size(), 1u);

  DeleteTodoCommand cmd(mgr, t);
  cmd.execute();

  EXPECT_EQ(mgr.getAllTodos().size(), 0u);
  EXPECT_FALSE(mgr.findById("del1").has_value());
}

TEST_F(CommandTest, DeleteTodoCommand_Undo_TodoRestored) {
  Todo t = makeTodo("del2", "UndoDelete");
  ASSERT_TRUE(mgr.addTodo(t));

  DeleteTodoCommand cmd(mgr, t);
  cmd.execute();
  ASSERT_EQ(mgr.getAllTodos().size(), 0u);

  cmd.undo();
  EXPECT_EQ(mgr.getAllTodos().size(), 1u);
  auto opt = mgr.findById("del2");
  ASSERT_TRUE(opt.has_value());
  EXPECT_EQ(opt->name, "UndoDelete");
}

TEST_F(CommandTest, DeleteTodoCommand_Execute_OtherTodosUnaffected) {
  Todo keep = makeTodo("keep1", "Keep");
  Todo remove = makeTodo("rem1", "Remove");
  ASSERT_TRUE(mgr.addTodo(keep));
  ASSERT_TRUE(mgr.addTodo(remove));

  DeleteTodoCommand cmd(mgr, remove);
  cmd.execute();

  EXPECT_EQ(mgr.getAllTodos().size(), 1u);
  EXPECT_TRUE(mgr.findById("keep1").has_value());
  EXPECT_FALSE(mgr.findById("rem1").has_value());
}

// ─── ToggleTodoCommand ───────────────────────────────────────────────────────

TEST_F(CommandTest, ToggleTodoCommand_Execute_FlipsStatus) {
  Todo t = makeTodo("tog1", "ToggleMe", false);
  ASSERT_TRUE(mgr.addTodo(t));

  ToggleTodoCommand cmd(mgr, t);
  cmd.execute();

  auto opt = mgr.findById("tog1");
  ASSERT_TRUE(opt.has_value());
  EXPECT_TRUE(opt->isComplete);
}

TEST_F(CommandTest, ToggleTodoCommand_Undo_RestoresStatus) {
  Todo t = makeTodo("tog2", "ToggleUndo", false);
  ASSERT_TRUE(mgr.addTodo(t));

  ToggleTodoCommand cmd(mgr, t);
  cmd.execute();   // false → true

  // Re-fetch state so undo operates on the correct (flipped) value
  auto afterExec = mgr.findById("tog2");
  ASSERT_TRUE(afterExec.has_value());
  ASSERT_TRUE(afterExec->isComplete);

  ToggleTodoCommand undoCmd(mgr, *afterExec);
  undoCmd.execute();  // true → false (undo is a second toggle)

  auto opt = mgr.findById("tog2");
  ASSERT_TRUE(opt.has_value());
  EXPECT_FALSE(opt->isComplete);
}

TEST_F(CommandTest, ToggleTodoCommand_AlreadyComplete_ExecuteUncompletes) {
  Todo t = makeTodo("tog3", "WasComplete", true);
  ASSERT_TRUE(mgr.addTodo(t));

  ToggleTodoCommand cmd(mgr, t);
  cmd.execute();  // true → false

  auto opt = mgr.findById("tog3");
  ASSERT_TRUE(opt.has_value());
  EXPECT_FALSE(opt->isComplete);
}

// ─── Entry point ────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new SuppressLogging());
  return RUN_ALL_TESTS();
}
