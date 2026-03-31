#include "StatsPanel.h"

StatsPanel::StatsPanel(WINDOW* w, int x, int y) : Panel(w, x, y) {}

void StatsPanel::setStats(int total, int completed) {
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

  std::string stringPush = std::to_string(pendingPush);
  std::string stringPull = std::to_string(pendingPull);

  // Justify to the right border with ~2 padding
  int startX = max_x - 10 - stringPull.size() - stringPush.size();

  wattron(win, COLOR_PAIR(1));
  mvwprintw(win, 1, startX, " + %s", stringPush.c_str());
  wattroff(win, COLOR_PAIR(1));
  wattron(win, COLOR_PAIR(2));
  wprintw(win, "  - %s ", stringPull.c_str());
  wattroff(win, COLOR_PAIR(2));
}
