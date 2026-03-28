#include "AddTodoCommand.h"

AddTodoCommand::AddTodoCommand(TodoManager& manager, const Todo& t) 
    : todoManager(manager), todo(t) {}

void AddTodoCommand::execute() {
    todoManager.addTodo(todo);
}

void AddTodoCommand::undo() {
    todoManager.removeTodo(todo);
}
