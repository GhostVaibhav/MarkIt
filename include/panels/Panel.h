#pragma once
#include <curses.h>
#include "i18n/I18nProvider.h"
#include <memory>

class Panel {
 public:
  Panel(WINDOW* win, int x, int y, std::shared_ptr<I18nProvider> i18n = nullptr);
  virtual ~Panel();

  virtual void render() = 0;

  void setPosition(int newX, int newY) {
    x = newX;
    y = newY;
  }
  void setWindow(WINDOW* newWin) { win = newWin; }

 protected:
  WINDOW* win;
  std::shared_ptr<I18nProvider> i18n;
  WINDOW* bottomBar = nullptr;
  int x;
  int y;
};
