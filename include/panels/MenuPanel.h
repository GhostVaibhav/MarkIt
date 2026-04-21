#pragma once
#include <string>
#include <vector>

#include "DimensionConfig.h"
#include "FullScreenPanel.h"
#include "LogoPanel.h"
#include "MenuAction.h"

class MenuPanel : public FullScreenPanel {
 public:
  explicit MenuPanel(WINDOW* win);
  ~MenuPanel();
  void render() override;
  void renderMenuItems();
  int promptSelection();

  int getMinWidth() const override { return Dimensions::MenuMinWidth; }
  int getMinHeight() const override { return Dimensions::MenuMinHeight; }

  void setMenuOptions(const std::vector<std::string>& options);
  void setSelectedIndex(unsigned int index);
  void setCredentials(const std::string& username, const std::string& pantryId);

 private:
  LogoPanel logoPanel;
  std::vector<std::string> options;
  unsigned int pointerIndex = 0;
  std::string curUser;
  std::string pantryId;

  WINDOW* titleWin = nullptr;
  WINDOW* menuWin = nullptr;
  void recreateWindows();
};
