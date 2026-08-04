#include "StatsPanel.h"

StatsPanel::StatsPanel(WINDOW* win, int x, int y, std::shared_ptr<I18nProvider> i18n) 
  : Panel(win, x, y, i18n) {}

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
  std::string pushFormat = i18n->get("push_stats").value_or("^ %s");
  pushFormat += "  ";
  std::string pullFormat = i18n->get("pull_stats").value_or("v %s");

  mvwprintw(targetWin, 1, startX, pushFormat.c_str(), stringPush.c_str());
  wattroff(targetWin, COLOR_PAIR(1));
  
  wattron(targetWin, COLOR_PAIR(2));
  wprintw(targetWin, pullFormat.c_str(), stringPull.c_str());
  wattroff(targetWin, COLOR_PAIR(2));
  
  // Notice we DON'T wrefresh here, as it's the caller's responsibility.
}
