#pragma once
#include <string>
#include <vector>

#include "DimensionConfig.h"
#include "FullScreenPanel.h"
#include "LoadingPanel.h"
#include "LogoPanel.h"
#include "Todo.h"

class MainMenuPanel : public FullScreenPanel {
 public:
  explicit MainMenuPanel(WINDOW* win);
  ~MainMenuPanel();
  void render() override;
  void renderList();
  void renderStats();

  void setTodos(const std::vector<Todo>& todos);
  void setSelectedIndex(int index);
  void setScroll(int topOffset);
  void setCredentials(const std::string& username, const std::string& pantryId);
  void setUpdateVersion(const std::string& version);

  int getMinWidth() const override { return Dimensions::MainMenuMinWidth; }
  int getMinHeight() const override { return Dimensions::MainMenuMinHeight; }
  int getVisibleRows() const;

 private:
  LoadingPanel loadingPanel;
  LogoPanel logoPanel;
  std::vector<Todo> todosList;

  int pointerIndex = 0;
  int moveFactor = 0;  // scroll offset
  std::string curUser;
  std::string pantryId;
  std::string updateVersion;  // Non-empty when update is ready

  std::string convertTimeToString(int epoch) const;

  WINDOW* todoUserName = nullptr;
  WINDOW* todoWindow = nullptr;
  WINDOW* todoBody = nullptr;
  void recreateWindows();
};
