#include "LanguageSelectionPanel.h"
#include <curses.h>

LanguageSelectionPanel::LanguageSelectionPanel(WINDOW* win, std::shared_ptr<I18nProvider> i18n)
    : FullScreenPanel(i18n), logoPanel(win, 0, 0, i18n) {
  if (i18n) {
    languages = i18n->getAvailableLanguages();
  } else {
    languages.push_back({"en", "English"});
  }
}

LanguageSelectionPanel::~LanguageSelectionPanel() {
  if (titleWin) delwin(titleWin);
  if (menuWin) delwin(menuWin);
}

void LanguageSelectionPanel::recreateWindows() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);
  int cw = getmaxx(win);
  int ch = getmaxy(win);
  
  if (titleWin) delwin(titleWin);
  if (menuWin) delwin(menuWin);

  int titleH = 8;
  int menuH = ch - titleH - 2;

  titleWin = derwin(win, titleH, cw - 4, 1, 2);
  menuWin = derwin(win, menuH, cw - 4, titleH + 1, 2);
}

void LanguageSelectionPanel::renderMenuItems() {
  werase(menuWin);
  int max_x = getmaxx(menuWin);

  for (size_t i = 0; i < languages.size(); ++i) {
    int y = i + 1;
    if (i == pointerIndex) {
      wattron(menuWin, A_REVERSE);
      mvwprintw(menuWin, y, 2, "-> %s", languages[i].second.c_str());
      wattroff(menuWin, A_REVERSE);
    } else {
      mvwprintw(menuWin, y, 2, "   %s", languages[i].second.c_str());
    }
  }

  wrefresh(menuWin);
}

void LanguageSelectionPanel::render() {
  recreateWindows();
  werase(win);
  box(win, 0, 0);

  int part = (getmaxx(win) - 36) / 2;
  if (part < 1) part = 1;
  logoPanel.setWindow(titleWin);
  logoPanel.setPosition(0, part);
  logoPanel.render();

  renderMenuItems();
  wrefresh(titleWin);
  wrefresh(win);

  std::string moveStr = i18n ? i18n->get("key_move").value_or("Move") : "Move";
  std::string selStr = i18n ? i18n->get("key_select").value_or("Select") : "Select";
  std::string exitStr = i18n ? i18n->get("key_exit").value_or("Exit") : "Exit";
  
  FullScreenPanel::refreshKeyBar(
      {{"Up/Dn", moveStr}, {"Enter", selStr}, {"Esc/q", exitStr}});
}

std::string LanguageSelectionPanel::promptSelection(std::function<void()> onIdle) {
  pointerIndex = 0;
  keypad(win, TRUE);
  nodelay(win, TRUE);

  while (true) {
    render();
    int ch;
    while ((ch = wgetch(win)) == ERR) {
      if (onIdle) onIdle();
      napms(50);
    }

    if (ch == KEY_UP && pointerIndex > 0) {
      pointerIndex--;
    } else if (ch == KEY_DOWN && pointerIndex < languages.size() - 1) {
      pointerIndex++;
    } else if (ch == 10) { // Enter
      return languages[pointerIndex].first;
    } else if (ch == 27 || ch == 'q' || ch == 'Q') { // ESC or q
      return "";
    }
  }
}
