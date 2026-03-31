#include "TodoDBManager.h"

#include <spdlog/spdlog.h>

TodoDBManager::TodoDBManager(const std::string& dbPath) : DBManager(dbPath) {}

void TodoDBManager::ensureTableExists(const User& user) {
  if (user.id.empty()) return;
  std::string sql = "CREATE TABLE IF NOT EXISTS records_" + user.id +
                    " ("
                    "id TEXT PRIMARY KEY, "
                    "name TEXT, "
                    "\"desc\" TEXT, "
                    "time INTEGER, "
                    "isComplete INTEGER)";
  db.exec(sql);
  spdlog::info("TodoDBManager: Ensured table exists for user {}", user.id);
}

std::vector<Todo> TodoDBManager::getTodos(const User& user) {
  std::vector<Todo> todos;
  if (user.id.empty()) return todos;
  ensureTableExists(user);

  std::string sql =
      "SELECT id, name, \"desc\", time, isComplete FROM records_" + user.id;
  SQLite::Statement query(db, sql);
  while (query.executeStep()) {
    Todo t;
    t.id = query.getColumn(0).getString();
    t.name = query.getColumn(1).getString();
    t.desc = query.getColumn(2).getString();
    t.time = query.getColumn(3).getInt();
    t.isComplete = query.getColumn(4).getInt() != 0;
    todos.push_back(t);
  }
  spdlog::info("TodoDBManager: Fetched {} todos for user {}", todos.size(),
               user.id);
  return todos;
}

bool TodoDBManager::addTodo(const User& user, const Todo& todo) {
  try {
    if (user.id.empty()) return false;
    ensureTableExists(user);
    std::string sql =
        "INSERT INTO records_" + user.id +
        " (id, name, \"desc\", time, isComplete) VALUES (?, ?, ?, ?, ?)";
    SQLite::Statement query(db, sql);
    query.bind(1, todo.id);
    query.bind(2, todo.name);
    query.bind(3, todo.desc);
    query.bind(4, todo.time);
    query.bind(5, todo.isComplete ? 1 : 0);
    query.exec();
    spdlog::info("TodoDBManager: Added todo {} for user {}", todo.id, user.id);
    return true;
  } catch (const std::exception& e) {
    spdlog::error("TodoDBManager: Failed to add todo for user {}: {}", user.id,
                  e.what());
    return false;
  }
}

bool TodoDBManager::removeTodo(const User& user, const Todo& todo) {
  try {
    if (user.id.empty()) return false;
    ensureTableExists(user);
    std::string sql = "DELETE FROM records_" + user.id + " WHERE id = ?";
    SQLite::Statement query(db, sql);
    query.bind(1, todo.id);
    query.exec();
    spdlog::info("TodoDBManager: Removed todo {} for user {}", todo.id,
                 user.id);
    return true;
  } catch (const std::exception& e) {
    spdlog::error("TodoDBManager: Failed to remove todo for user {}: {}",
                  user.id, e.what());
    return false;
  }
}

bool TodoDBManager::toggleTodo(const User& user, const Todo& todo) {
  try {
    if (user.id.empty()) return false;
    ensureTableExists(user);
    std::string sql =
        "UPDATE records_" + user.id + " SET isComplete = ? WHERE id = ?";
    SQLite::Statement query(db, sql);
    query.bind(1, todo.isComplete ? 1 : 0);
    query.bind(2, todo.id);
    query.exec();
    spdlog::info("TodoDBManager: Toggled todo {} for user {}", todo.id,
                 user.id);
    return true;
  } catch (const std::exception& e) {
    spdlog::error("TodoDBManager: Failed to toggle todo for user {}: {}",
                  user.id, e.what());
    return false;
  }
}
