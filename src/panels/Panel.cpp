#include "Panel.h"

Panel::Panel(WINDOW* win, int x, int y, std::shared_ptr<I18nProvider> i18n)
    : win(win), i18n(i18n), bottomBar(nullptr), x(x), y(y) {}

Panel::~Panel() {
  if (bottomBar) delwin(bottomBar);
}
