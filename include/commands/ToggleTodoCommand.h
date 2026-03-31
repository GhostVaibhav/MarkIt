#pragma once
#include "ITodoCommand.h"
#include "TodoManager.h"

class ToggleTodoCommand : public ITodoCommand {
 public:
  ToggleTodoCommand(TodoManager& manager, const Todo& todo);
  void execute() override;
  void undo() override;

 private:
  TodoManager& todoManager;
  Todo todo;
};
