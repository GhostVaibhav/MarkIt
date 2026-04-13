#include "FullScreenPanel.h"

#include "BottomBarHelper.h"

FullScreenPanel::FullScreenPanel() : win(stdscr), bottomBar(nullptr) {
  statsPanel = new StatsPanel(win, 0, 0);
}

FullScreenPanel::~FullScreenPanel() {
  if (bottomBar) delwin(bottomBar);
  if (statsPanel) delete statsPanel;
}

void FullScreenPanel::refreshKeyBar(
    const std::vector<std::pair<std::string, std::string>>& keys) {
  bottomBar = drawBottomBar(bottomBar, keys);
}

void FullScreenPanel::clearKeyBar() {
  if (bottomBar) {
    delwin(bottomBar);
    bottomBar = nullptr;
  }
}

void FullScreenPanel::show() {
  if (!checkSize()) {
    renderSizeWarning();
    return;
  }
  wclear(win);
  wrefresh(win);
  render();
}

void FullScreenPanel::handleResize() {
  resize_term(0, 0);
  clear();
  refresh();
  wclear(win);
  show();
}

bool FullScreenPanel::checkSize() {
  int max_y, max_x;
  getmaxyx(win, max_y, max_x);
  return (max_x >= getMinWidth() && max_y >= getMinHeight());
}

void FullScreenPanel::renderSizeWarning() {
  wclear(win);
  int max_y, max_x;
  getmaxyx(win, max_y, max_x);
  mvwprintw(win, max_y / 2, (max_x - 22) / 2, "Please enlarge terminal");
  wrefresh(win);
}
