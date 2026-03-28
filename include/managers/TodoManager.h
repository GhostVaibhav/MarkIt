#pragma once
#include "Todo.h"
#include "TodoDBManager.h"
#include "User.h"
#include <vector>
#include <optional>
#include <string>

class TodoManager {
public:
    explicit TodoManager(const std::string& dbPath);

    void setCurrentUser(const User& user);

    bool addTodo(const Todo& todo);
    bool removeTodo(const Todo& todo);
    bool toggleTodo(const Todo& todo);
    std::vector<Todo> getAllTodos();

    bool validate(const Todo& todo) const;
    std::optional<Todo> findById(const std::string& id);
    std::vector<Todo> search(const std::string& query);
    
    std::vector<Todo> todos;

private:
    TodoDBManager todoDBManager;
    std::optional<User> currentUser;
    void refreshTodos();
};
