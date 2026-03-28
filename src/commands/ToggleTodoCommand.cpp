#include "ToggleTodoCommand.h"

ToggleTodoCommand::ToggleTodoCommand(TodoManager& manager, const Todo& t) 
    : todoManager(manager), todo(t) {}

void ToggleTodoCommand::execute() {
    todoManager.toggleTodo(todo);
}

void ToggleTodoCommand::undo() {
    todoManager.toggleTodo(todo);
}
