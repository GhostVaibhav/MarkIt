#pragma once
#include <curses.h>

class Panel {
 public:
  Panel(WINDOW* win, int x, int y);
  virtual ~Panel();

  virtual void render() = 0;

  void setPosition(int newX, int newY) {
    x = newX;
    y = newY;
  }
  void setWindow(WINDOW* newWin) { win = newWin; }

 protected:
  WINDOW* win;
  WINDOW* bottomBar = nullptr;
  int x;
  int y;
};
