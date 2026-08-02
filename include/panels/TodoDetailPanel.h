#pragma once
#include <curses.h>

#include <string>
#include <vector>

#include <functional>

#include "DimensionConfig.h"
#include "FullScreenPanel.h"
#include "LogoPanel.h"
#include "Todo.h"

enum class TodoDetailAction { Toggle, Delete, Back, QuitApp, OpenMenu, EditTodo };

class TodoDetailPanel : public FullScreenPanel {
 public:
  explicit TodoDetailPanel(std::shared_ptr<I18nProvider> i18n = nullptr);
  ~TodoDetailPanel() override;
  void setTodo(const Todo& todo);
  void setCredentials(const std::string& username, const std::string& id);
  void render() override;
  void renderSyncStateOnly() override;
  TodoDetailAction promptAction(std::function<void()> onIdle = nullptr);
  
  void recreateWindows();

  int getMinWidth() const override { return Dimensions::TodoDetailMinWidth; }
  int getMinHeight() const override { return Dimensions::TodoDetailMinHeight; }

 private:
  void renderContent();
  LogoPanel logoPanel;
  Todo currentTodo;
  std::string curUser;
  std::string pantryId;
  
  WINDOW* todoUserName;
  WINDOW* contentWin;

  int scrollOffset = 0;   // lines scrolled down in the content window

  // Build all virtual content lines for the current todo (for scroll calculations)
  std::vector<std::string> buildContentLines(int lineWidth) const;
};
