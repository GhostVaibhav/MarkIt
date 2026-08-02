#pragma once
#include <string>
#include <vector>

#include <functional>

#include "DimensionConfig.h"
#include "FullScreenPanel.h"
#include "LogoPanel.h"
#include "MenuAction.h"

class MenuPanel : public FullScreenPanel {
 public:
  explicit MenuPanel(WINDOW* win, std::shared_ptr<I18nProvider> i18n = nullptr);
  ~MenuPanel();
  void render() override;
  void renderSyncStateOnly() override;
  void renderMenuItems();
  int promptSelection(std::function<void()> onIdle = nullptr);

  int getMinWidth() const override { return Dimensions::MenuMinWidth; }
  int getMinHeight() const override { return Dimensions::MenuMinHeight; }

  void setMenuOptions(const std::vector<std::string>& opts);
  void setSelectedIndex(unsigned int index);
  std::string getOption(int index) const {
    if (index >= 0 && index < (int)options.size()) return options[index];
    return "";
  }
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
