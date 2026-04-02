#pragma once
#include <string>
#include <curses.h>

#include "Panel.h"

class AddTodoPanel : public Panel {
 public:
  explicit AddTodoPanel();
  ~AddTodoPanel() override;

  void render() override;

  void promptInput();
  void resizeEvent();

  std::string getEnteredName() const;
  std::string getEnteredDesc() const;

 private:
  std::string name;
  std::string desc;
  
  // Added to manage state during resizes safely
  WINDOW* addWin = nullptr; 
  
  void recreateWindows();
  std::string captureInput(bool isNameField);
};
