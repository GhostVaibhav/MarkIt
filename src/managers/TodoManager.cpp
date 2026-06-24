#include "TodoManager.h"

#include <spdlog/spdlog.h>

#include <algorithm>

TodoManager::TodoManager(const std::string& dbPath) : todoDBManager(dbPath) {}

void TodoManager::setCurrentUser(const User& user) {
  std::lock_guard<std::recursive_mutex> lock(todoMtx);
  spdlog::info("TodoManager: Bound active user '{}'", user.name);
  currentUser = user;
  refreshTodos();
}

void TodoManager::refreshTodos() {
  std::lock_guard<std::recursive_mutex> lock(todoMtx);
  if (currentUser) {
    try {
      todos = todoDBManager.getTodos(*currentUser);
      spdlog::info("TodoManager: Refreshed {} todos from DB cache", todos.size());
    } catch (const std::exception& e) {
      spdlog::error("TodoManager: Database error while refreshing todos: {}", e.what());
    }
  } else {
    todos.clear();
    spdlog::info("TodoManager: Cleared memory cache (no bound user)");
  }
}

bool TodoManager::addTodo(const Todo& todo) {
  std::lock_guard<std::recursive_mutex> lock(todoMtx);
  if (!currentUser || !validate(todo)) return false;
  if (todoDBManager.addTodo(*currentUser, todo)) {
    todos.push_back(todo);
    spdlog::info("TodoManager: Added and cached new todo: '{}'", todo.id);
    return true;
  }
  spdlog::warn("TodoManager: Failed to securely DB persist new todo: '{}'",
               todo.id);
  return false;
}

bool TodoManager::removeTodo(const Todo& todo) {
  std::lock_guard<std::recursive_mutex> lock(todoMtx);
  if (!currentUser) return false;
  if (todoDBManager.removeTodo(*currentUser, todo)) {
    auto it = std::remove_if(todos.begin(), todos.end(),
                             [&](const Todo& t) { return t.id == todo.id; });
    if (it != todos.end()) {
      todos.erase(it, todos.end());
      spdlog::info("TodoManager: Removed todo from cache: '{}'", todo.id);
    }
    return true;
  }
  spdlog::warn("TodoManager: Failed to DB remove todo: '{}'", todo.id);
  return false;
}

bool TodoManager::toggleTodo(const Todo& todo) {
  std::lock_guard<std::recursive_mutex> lock(todoMtx);
  if (!currentUser) return false;

  Todo updatedTodo = todo;
  updatedTodo.isComplete = !updatedTodo.isComplete;

  if (todoDBManager.toggleTodo(*currentUser, updatedTodo)) {
    auto it = std::find_if(todos.begin(), todos.end(),
                           [&](const Todo& t) { return t.id == todo.id; });
    if (it != todos.end()) {
      it->isComplete = updatedTodo.isComplete;
      spdlog::info("TodoManager: Toggled todo status: '{}' -> {}", todo.id,
                   updatedTodo.isComplete);
    }
    return true;
  }
  spdlog::warn("TodoManager: Failed to DB toggle todo: '{}'", todo.id);
  return false;
}

bool TodoManager::updateTodo(const Todo& todo) {
  std::lock_guard<std::recursive_mutex> lock(todoMtx);
  if (!currentUser || !validate(todo)) return false;

  if (todoDBManager.updateTodo(*currentUser, todo)) {
    auto it = std::find_if(todos.begin(), todos.end(),
                           [&](const Todo& t) { return t.id == todo.id; });
    if (it != todos.end()) {
      *it = todo;
      spdlog::info("TodoManager: Updated todo in cache: '{}'", todo.id);
    }
    return true;
  }
  spdlog::warn("TodoManager: Failed to DB update todo: '{}'", todo.id);
  return false;
}

std::vector<Todo> TodoManager::getAllTodos() {
  std::lock_guard<std::recursive_mutex> lock(todoMtx);
  refreshTodos();
  return todos;
}

bool TodoManager::validate(const Todo& todo) const {
  return !todo.name.empty();
}

std::optional<Todo> TodoManager::findById(const std::string& id) {
  std::lock_guard<std::recursive_mutex> lock(todoMtx);
  auto it = std::find_if(todos.begin(), todos.end(),
                         [&](const Todo& t) { return t.id == id; });
  if (it != todos.end()) return *it;
  return std::nullopt;
}

std::vector<Todo> TodoManager::search(const std::string& query) {
  std::lock_guard<std::recursive_mutex> lock(todoMtx);
  std::vector<Todo> results;
  for (const auto& t : todos) {
    if (t.name.find(query) != std::string::npos ||
        t.desc.find(query) != std::string::npos) {
      results.push_back(t);
    }
  }
  return results;
}
