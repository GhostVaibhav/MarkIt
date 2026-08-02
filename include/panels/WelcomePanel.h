#pragma once
#include <string>

#include "DimensionConfig.h"
#include "FullScreenPanel.h"
#include "LogoPanel.h"

class WelcomePanel : public FullScreenPanel {
 public:
  explicit WelcomePanel(WINDOW* win, std::shared_ptr<I18nProvider> i18n = nullptr);
  void render() override;
  /** Blocks until a key is read. Returns false if the user chose to quit (q,
   * Esc, Ctrl+C). */
  bool waitForContinue();

  void setCode(int code);
  void setUsername(const std::string& username);
  int getCode() const;

  int getMinWidth() const override { return Dimensions::WelcomeMinWidth; }
  int getMinHeight() const override { return Dimensions::WelcomeMinHeight; }

 private:
  LogoPanel logoPanel;
  int code = 0;
  std::string curUser;
};
