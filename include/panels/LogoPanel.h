#pragma once
#include "Panel.h"

class LogoPanel : public Panel {
 public:
  LogoPanel(WINDOW* win, int x, int y, std::shared_ptr<I18nProvider> i18n = nullptr);
  void render() override;
};
