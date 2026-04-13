#pragma once
#include <string>
#include <curses.h>

#include "DimensionConfig.h"
#include "FullScreenPanel.h"
#include "LogoPanel.h"

class AddTodoPanel : public FullScreenPanel {
 public:
  explicit AddTodoPanel();
  ~AddTodoPanel() override;

  void render() override;

  void promptInput();

  int getMinWidth() const override { return Dimensions::AddTodoMinWidth; }
  int getMinHeight() const override { return Dimensions::AddTodoMinHeight; }

  std::string getEnteredName() const;
  std::string getEnteredDesc() const;

  void setCredentials(const std::string& username, const std::string& id);

 private:
  std::string name;
  std::string desc;
  
  std::string curUser;
  std::string pantryId;
  
  LogoPanel logoPanel;
  WINDOW* titleWin = nullptr; 
  WINDOW* contentWin = nullptr; 
  
  void recreateWindows();
  std::string captureInput(bool isNameField);
};
