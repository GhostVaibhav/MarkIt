#pragma once
#include <curses.h>

#include <string>
#include <vector>

#include "DimensionConfig.h"
#include "FullScreenPanel.h"
#include "LogoPanel.h"
#include "Todo.h"

enum class TodoDetailAction { Toggle, Delete, Back, QuitApp, OpenMenu };

class TodoDetailPanel : public FullScreenPanel {
 public:
  explicit TodoDetailPanel();
  ~TodoDetailPanel() override;
  void setTodo(const Todo& todo);
  void setCredentials(const std::string& username, const std::string& id);
  void render() override;
  TodoDetailAction promptAction();
  
  void recreateWindows();

  int getMinWidth() const override { return Dimensions::TodoDetailMinWidth; }
  int getMinHeight() const override { return Dimensions::TodoDetailMinHeight; }

 private:
  LogoPanel logoPanel;
  Todo currentTodo;
  std::string curUser;
  std::string pantryId;
  
  WINDOW* todoUserName;
  WINDOW* contentWin;
};
