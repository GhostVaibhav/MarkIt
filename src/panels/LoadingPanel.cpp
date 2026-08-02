#include "LoadingPanel.h"

LoadingPanel::LoadingPanel(WINDOW* w, std::shared_ptr<I18nProvider> i18n) 
    : FullScreenPanel(i18n), logoPanel(w, 0, 0, i18n) {
  if (i18n) {
    loadingText = i18n->get("loading_msg");
  }
}

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
  std::string waitStr = i18n ? i18n->get("key_please_wait") : "Please wait";
  FullScreenPanel::refreshKeyBar({{"---", waitStr}});
}
