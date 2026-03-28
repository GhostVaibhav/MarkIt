#include "DeleteTodoCommand.h"

DeleteTodoCommand::DeleteTodoCommand(TodoManager& manager, const Todo& t) 
    : todoManager(manager), todo(t) {}

void DeleteTodoCommand::execute() {
    todoManager.removeTodo(todo);
}

void DeleteTodoCommand::undo() {
    todoManager.addTodo(todo);
}
