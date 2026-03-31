#pragma once

class ITodoCommand {
 public:
  virtual ~ITodoCommand() = default;
  virtual void execute() = 0;
  virtual void undo() = 0;
};
