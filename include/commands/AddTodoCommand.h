#pragma once
#include "ITodoCommand.h"
#include "TodoManager.h"

class AddTodoCommand : public ITodoCommand {
public:
    AddTodoCommand(TodoManager& manager, const Todo& todo);
    void execute() override;
    void undo() override;
private:
    TodoManager& todoManager;
    Todo todo;
};
