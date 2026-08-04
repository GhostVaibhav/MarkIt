#pragma once
#include <string>
#include <vector>
#include <utility>
#include <functional>

#include "FullScreenPanel.h"
#include "LogoPanel.h"
#include "i18n/I18nProvider.h"

class LanguageSelectionPanel : public FullScreenPanel {
 public:
  explicit LanguageSelectionPanel(WINDOW* win, std::shared_ptr<I18nProvider> i18n = nullptr);
  ~LanguageSelectionPanel();
  
  void render() override;
  std::string promptSelection(std::function<void()> onIdle = nullptr);

  int getMinWidth() const override { return 78; }
  int getMinHeight() const override { return 20; }

 private:
  LogoPanel logoPanel;
  std::vector<std::pair<std::string, std::string>> languages;
  unsigned int pointerIndex = 0;

  WINDOW* titleWin = nullptr;
  WINDOW* menuWin = nullptr;
  
  void recreateWindows();
  void renderMenuItems();
};
