#pragma once
#include <string>
#include <vector>

#include "FullScreenPanel.h"
#include "LoadingPanel.h"
#include "LogoPanel.h"
#include "StatsPanel.h"
#include "Todo.h"

class MainMenuPanel : public FullScreenPanel {
 public:
  explicit MainMenuPanel(WINDOW* win);
  ~MainMenuPanel();
  void render() override;

  void setTodos(const std::vector<Todo>& todos);
  void setSelectedIndex(int index);
  void setScroll(int topOffset);
  void setCredentials(const std::string& username, const std::string& pantryId);
  void setStats(int total, int completed);
  void setSyncStatus(int pendingPush, int pendingPull);

 private:
  LoadingPanel loadingPanel;
  LogoPanel logoPanel;
  StatsPanel statsPanel;
  std::vector<Todo> todosList;

  int pointerIndex = 0;
  int moveFactor = 0;  // scroll offset
  std::string curUser;
  std::string pantryId;

  std::string convertTimeToString(int epoch) const;

  WINDOW* todoUserName = nullptr;
  WINDOW* todoWindow = nullptr;
  WINDOW* todoBody = nullptr;
  void recreateWindows();
};
