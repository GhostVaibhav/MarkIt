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
  // Fallback to legacy render if called
  render(win);
}

void StatsPanel::render(WINDOW* targetWin) {
  if (!targetWin) return;
  int max_y, max_x;
  getmaxyx(targetWin, max_y, max_x);
  (void)max_y;

  std::string stringPush = std::to_string(pendingPush);
  std::string stringPull = std::to_string(pendingPull);

  int width = 8 + (int) stringPull.size() + (int) stringPush.size();
  int startX = max_x - width - 1;
  if (startX < 0) startX = 0;

  wattron(targetWin, COLOR_PAIR(1));
  mvwprintw(targetWin, 1, startX, " + %s", stringPush.c_str());
  wattroff(targetWin, COLOR_PAIR(1));
  
  wattron(targetWin, COLOR_PAIR(2));
  wprintw(targetWin, "  - %s ", stringPull.c_str());
  wattroff(targetWin, COLOR_PAIR(2));
  
  // Notice we DON'T wrefresh here, as it's the caller's responsibility.
}
