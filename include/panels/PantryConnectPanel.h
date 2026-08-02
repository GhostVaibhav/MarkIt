#pragma once
#include <string>

#include <functional>

#include "DimensionConfig.h"
#include "FullScreenPanel.h"
#include "LogoPanel.h"

class PantryConnectPanel : public FullScreenPanel {
 public:
  explicit PantryConnectPanel(WINDOW* win, std::shared_ptr<I18nProvider> i18n = nullptr);
  ~PantryConnectPanel();

  void render() override;
  void promptInput(std::function<void()> onIdle = nullptr);
  std::string getEnteredKey() const;
  void setCredentials(const std::string& username, const std::string& id);

  int getMinWidth() const override { return Dimensions::PantryConnectMinWidth; }
  int getMinHeight() const override { return Dimensions::PantryConnectMinHeight; }

 private:
  LogoPanel logoPanel;
  std::string apiKey;
  std::string curUser;
  std::string pantryId;

  WINDOW* titleWin = nullptr;
  WINDOW* contentWin = nullptr;

  void recreateWindows();
};
