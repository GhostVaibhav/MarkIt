#pragma once
#include <string>
#include <vector>

#include "FullScreenPanel.h"
#include "LogoPanel.h"
#include "MenuAction.h"
#include "StatsPanel.h"

class MenuPanel : public FullScreenPanel {
 public:
  explicit MenuPanel(WINDOW* win);
  ~MenuPanel();
  void render() override;
  int promptSelection();

  void setMenuOptions(const std::vector<std::string>& options);
  void setSelectedIndex(unsigned int index);
  void setCredentials(const std::string& username, const std::string& pantryId);

 private:
  LogoPanel logoPanel;
  StatsPanel statsPanel;
  std::vector<std::string> options;
  unsigned int pointerIndex = 0;
  std::string curUser;
  std::string pantryId;

  WINDOW* titleWin = nullptr;
  WINDOW* menuWin = nullptr;
  void recreateWindows();
};
