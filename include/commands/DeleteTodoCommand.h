#pragma once
#include "ITodoCommand.h"
#include "TodoManager.h"

class DeleteTodoCommand : public ITodoCommand {
public:
    DeleteTodoCommand(TodoManager& manager, const Todo& todo);
    void execute() override;
    void undo() override;
private:
    TodoManager& todoManager;
    Todo todo;
};
