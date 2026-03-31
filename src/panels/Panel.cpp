#include "Panel.h"

Panel::Panel(WINDOW* win, int x, int y) : win(win), x(x), y(y) {}

Panel::~Panel() {
  if (bottomBar) delwin(bottomBar);
}
