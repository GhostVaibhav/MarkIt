#include "TodoManager.h"
#include <spdlog/spdlog.h>
#include <algorithm>

TodoManager::TodoManager(const std::string& dbPath) : todoDBManager(dbPath) {}

void TodoManager::setCurrentUser(const User& user) {
    spdlog::info("TodoManager: Bound active user '{}'", user.name);
    currentUser = user;
    refreshTodos();
}

void TodoManager::refreshTodos() {
    if (currentUser) {
        todos = todoDBManager.getTodos(*currentUser);
        spdlog::info("TodoManager: Refreshed {} todos from DB cache", todos.size());
    } else {
        todos.clear();
        spdlog::info("TodoManager: Cleared memory cache (no bound user)");
    }
}

bool TodoManager::addTodo(const Todo& todo) {
    if (!currentUser || !validate(todo)) return false;
    if (todoDBManager.addTodo(*currentUser, todo)) {
        todos.push_back(todo);
        spdlog::info("TodoManager: Added and cached new todo: '{}'", todo.id);
        return true;
    }
    spdlog::warn("TodoManager: Failed to securely DB persist new todo: '{}'", todo.id);
    return false;
}

bool TodoManager::removeTodo(const Todo& todo) {
    if (!currentUser) return false;
    if (todoDBManager.removeTodo(*currentUser, todo)) {
        auto it = std::remove_if(todos.begin(), todos.end(), [&](const Todo& t) { return t.id == todo.id; });
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
    if (!currentUser) return false;
    
    Todo updatedTodo = todo;
    updatedTodo.isComplete = !updatedTodo.isComplete;
    
    if (todoDBManager.toggleTodo(*currentUser, updatedTodo)) {
        auto it = std::find_if(todos.begin(), todos.end(), [&](const Todo& t) { return t.id == todo.id; });
        if (it != todos.end()) {
            it->isComplete = updatedTodo.isComplete;
            spdlog::info("TodoManager: Toggled todo status: '{}' -> {}", todo.id, updatedTodo.isComplete);
        }
        return true;
    }
    spdlog::warn("TodoManager: Failed to DB toggle todo: '{}'", todo.id);
    return false;
}

std::vector<Todo> TodoManager::getAllTodos() {
    refreshTodos();
    return todos;
}

bool TodoManager::validate(const Todo& todo) const {
    return !todo.name.empty();
}

std::optional<Todo> TodoManager::findById(const std::string& id) {
    auto it = std::find_if(todos.begin(), todos.end(), [&](const Todo& t) { return t.id == id; });
    if (it != todos.end()) return *it;
    return std::nullopt;
}

std::vector<Todo> TodoManager::search(const std::string& query) {
    std::vector<Todo> results;
    for (const auto& t : todos) {
        if (t.name.find(query) != std::string::npos || t.desc.find(query) != std::string::npos) {
            results.push_back(t);
        }
    }
    return results;
}
