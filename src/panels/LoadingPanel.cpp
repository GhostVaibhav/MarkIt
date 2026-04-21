#include "LoadingPanel.h"

LoadingPanel::LoadingPanel(WINDOW* w) : FullScreenPanel(), logoPanel(w, 0, 0) {}

void LoadingPanel::setLoadingText(const std::string& text) {
  loadingText = text;
}

void LoadingPanel::render() {
  int max_y, max_x;
  getmaxyx(win, max_y, max_x);
  int part = (max_x - 36) / 2;
  int half = ((max_y - 4) / 2) - 1;

  logoPanel.setWindow(win);
  logoPanel.setPosition(half, part);
  logoPanel.render();

  mvwprintw(win, max_y - 2, (max_x - (int) loadingText.size()) / 2, "%s",
            loadingText.c_str());
  wrefresh(win);
  FullScreenPanel::refreshKeyBar({{"---", "Please wait"}});
}
