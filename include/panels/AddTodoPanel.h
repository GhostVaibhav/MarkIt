#pragma once
#include <string>

#include "Panel.h"

class AddTodoPanel : public Panel {
 public:
  explicit AddTodoPanel();
  ~AddTodoPanel() override;

  void render() override;

  void promptInput();

  std::string getEnteredName() const;
  std::string getEnteredDesc() const;

 private:
  std::string name;
  std::string desc;
};
