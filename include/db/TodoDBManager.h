#pragma once
#include <vector>

#include "DBManager.h"
#include "Todo.h"
#include "User.h"

class TodoDBManager : public DBManager {
 public:
  explicit TodoDBManager(const std::string& dbPath);

  std::vector<Todo> getTodos(const User& user);
  bool addTodo(const User& user, const Todo& todo);
  bool removeTodo(const User& user, const Todo& todo);
  bool toggleTodo(const User& user, const Todo& todo);
  bool updateTodo(const User& user, const Todo& todo);

 private:
  void ensureTableExists(const User& user);
};
