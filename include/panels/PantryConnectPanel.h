#pragma once
#include <string>

#include "FullScreenPanel.h"
#include "LogoPanel.h"

class PantryConnectPanel : public FullScreenPanel {
 public:
  explicit PantryConnectPanel(WINDOW* win);
  ~PantryConnectPanel();

  void render() override;
  void promptInput();
  std::string getEnteredKey() const;

 private:
  LogoPanel logoPanel;
  std::string apiKey;
  WINDOW* contentWin = nullptr;

  void recreateWindows();
};
