#include "AddTodoPanel.h"

#include <curses.h>

#include "BottomBarHelper.h"

AddTodoPanel::AddTodoPanel() : Panel(stdscr, 0, 0) {}

AddTodoPanel::~AddTodoPanel() {
  if (bottomBar) delwin(bottomBar);
}

void AddTodoPanel::render() {}

void AddTodoPanel::promptInput() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  bottomBar = drawBottomBar(bottomBar, {{"Enter", "Next field"}});

  WINDOW* addWin = newwin(10, 60, (max_y - 10) / 2, (max_x - 60) / 2);
  box(addWin, 0, 0);

  char nameBuf[128] = {0};
  char descBuf[256] = {0};

  mvwprintw(addWin, 2, 2, "Enter Todo Name:");
  mvwprintw(addWin, 5, 2, "Enter Todo Description:");
  wrefresh(addWin);

  echo();
  curs_set(1);

  mvwgetnstr(addWin, 3, 2, nameBuf, 127);
  mvwgetnstr(addWin, 6, 2, descBuf, 255);

  noecho();
  curs_set(0);
  delwin(addWin);
  if (bottomBar) {
    delwin(bottomBar);
    bottomBar = nullptr;
  }

  name = std::string(nameBuf);
  desc = std::string(descBuf);
}

std::string AddTodoPanel::getEnteredName() const { return name; }
std::string AddTodoPanel::getEnteredDesc() const { return desc; }
