#include "StatsPanel.h"

StatsPanel::StatsPanel(WINDOW* w, int x, int y) : Panel(w, x, y) {}

StatsPanel::~StatsPanel() {
  if (statsWin) delwin(statsWin);
}

void StatsPanel::setStats(size_t total, int completed) {
  totalTodos = total;
  completedTodos = completed;
}

void StatsPanel::setSyncStatus(int push, int pull) {
  pendingPush = push;
  pendingPull = pull;
}

void StatsPanel::render() {
  int max_y, max_x;
  getmaxyx(win, max_y, max_x);
  (void)max_y;

  std::string stringPush = std::to_string(pendingPush);
  std::string stringPull = std::to_string(pendingPull);

  int width = 10 + (int) stringPull.size() + (int) stringPush.size();
  int startX = max_x - width - 2;
  if (startX < 0) startX = 0;

  if (!statsWin) {
    statsWin = newwin(3, width + 2, 0, startX);
  } else {
    // move back to 0,0 temporarily to avoid out of bounds
    mvwin(statsWin, 0, 0);
#ifdef _WIN32
    resize_window(statsWin, 3, width + 2);
#else
    wresize(statsWin, 3, width + 2);
#endif
    mvwin(statsWin, 0, startX);
  }

  wclear(statsWin);

  wattron(statsWin, COLOR_PAIR(1));
  mvwprintw(statsWin, 1, 0, " + %s", stringPush.c_str());
  wattroff(statsWin, COLOR_PAIR(1));
  
  wattron(statsWin, COLOR_PAIR(2));
  wprintw(statsWin, "  - %s ", stringPull.c_str());
  wattroff(statsWin, COLOR_PAIR(2));
  
  wrefresh(statsWin);
}
