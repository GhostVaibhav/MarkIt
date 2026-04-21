#pragma once
#include <curses.h>

#include <string>
#include <vector>

inline WINDOW *drawBottomBar(
    WINDOW *oldBar,
    const std::vector<std::pair<std::string, std::string>> &keys) {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  if (oldBar) {
    delwin(oldBar);
  }

  int bar_y = (max_y > 0) ? max_y - 1 : 0;
  int bar_w = (max_x > 0) ? max_x : 1;

  WINDOW *bar = newwin(1, bar_w, bar_y, 0);
  wclear(bar);

  int currentX = 0;
  for (const auto &k : keys) {
    if (currentX >= bar_w) break;

    if (has_colors()) wattron(bar, COLOR_PAIR(6));
    mvwprintw(bar, 0, currentX, " %s ", k.first.c_str());
    currentX += static_cast<int>(k.first.length()) + 2;

    if (has_colors()) {
      wattroff(bar, COLOR_PAIR(6));
      wattron(bar, COLOR_PAIR(7));
    } else {
      wattron(bar, A_REVERSE);
    }

    if (currentX < bar_w) {
      mvwprintw(bar, 0, currentX, " %s ", k.second.c_str());
    }
    currentX += static_cast<int>(k.second.length()) + 2;

    if (has_colors())
      wattroff(bar, COLOR_PAIR(7));
    else
      wattroff(bar, A_REVERSE);

    currentX += 1;
  }
  wrefresh(bar);
  return bar;
}
