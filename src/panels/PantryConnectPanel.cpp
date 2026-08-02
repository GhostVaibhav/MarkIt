#include "PantryConnectPanel.h"
#include "utils/StringUtils.h"

#include <curses.h>

PantryConnectPanel::PantryConnectPanel(WINDOW* w, std::shared_ptr<I18nProvider> i18n)
    : FullScreenPanel(i18n), logoPanel(w, 0, 0, i18n) {}

PantryConnectPanel::~PantryConnectPanel() {
  if (titleWin) delwin(titleWin);
  if (contentWin) delwin(contentWin);
}

void PantryConnectPanel::setCredentials(const std::string& username, const std::string& id) {
  curUser = username;
  pantryId = id;
}

void PantryConnectPanel::recreateWindows() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  int safe_w = (max_x > 2) ? max_x - 2 : 1;
  int safe_h_win = (max_y > 13) ? max_y - 13 : 1;

  if (!titleWin) {
    titleWin = newwin(10, safe_w, 1, 1);
    contentWin = newwin(safe_h_win, safe_w, 11, 1);
  } else {
    mvwin(titleWin, 0, 0);
    mvwin(contentWin, 0, 0);

#ifdef _WIN32
    resize_window(titleWin, 10, safe_w);
    resize_window(contentWin, safe_h_win, safe_w);
#else
    wresize(titleWin, 10, safe_w);
    wresize(contentWin, safe_h_win, safe_w);
#endif

    mvwin(titleWin, 1, 1);
    mvwin(contentWin, 11, 1);
  }
}

void PantryConnectPanel::render() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  if (win && win != stdscr) {
#ifdef _WIN32
    resize_window(win, max_y, max_x);
#else
    wresize(win, max_y, max_x);
#endif
  }

  recreateWindows();

  wclear(titleWin);
  wclear(contentWin);

  box(titleWin, 0, 0);
  box(contentWin, 0, 0);

  int part = (getmaxx(titleWin) - 81) / 4;
  if (part <= 0) part = 2; // enforce minimum padding

  logoPanel.setWindow(titleWin);
  logoPanel.setPosition(1, part);
  logoPanel.render();
  
  FullScreenPanel::renderSyncIndicator(titleWin);
  
  if (statsPanel) {
    statsPanel->render(titleWin);
  }

  int u_x = getmaxx(titleWin) - 50;
  if (u_x < part + 45) u_x = part + 45;
  int remainingW = getmaxx(titleWin) - u_x - 1;
  if (remainingW < 3) remainingW = 3;

  std::string userLabel = i18n ? i18n->get("username_label") : "Username: ";
  std::string dispUser = userLabel + curUser;
  mvwprintw(titleWin, 3, u_x, "%s", StringUtils::truncateString(dispUser, remainingW).c_str());
  if (!pantryId.empty() && pantryId != "None") {
    std::string idLabel = i18n ? i18n->get("pantry_id_label") : "Pantry ID: ";
    std::string dispId = idLabel + pantryId;
    mvwprintw(titleWin, 5, u_x, "%s", StringUtils::truncateString(dispId, remainingW).c_str());
  }

  std::string apiPrompt = i18n ? i18n->get("enter_pantry_key") : "Enter Pantry API Key (or empty to cancel):";
  mvwprintw(contentWin, 2, 2, "%s", apiPrompt.c_str());
  mvwprintw(contentWin, 4, 2, "%s", apiKey.c_str());

  wrefresh(titleWin);
  wrefresh(contentWin);
  std::string submitStr = i18n ? i18n->get("key_submit") : "Submit";
  std::string cancelStr = i18n ? i18n->get("key_cancel") : "Cancel";
  std::string exitStr = i18n ? i18n->get("key_exit") : "Exit";
  FullScreenPanel::refreshKeyBar({{"Enter", submitStr}, {"empty", cancelStr}, {"^C", exitStr}});
}

void PantryConnectPanel::promptInput(std::function<void()> onIdle) {
  noecho();
  curs_set(1);
  apiKey = "";

  int ch;
  keypad(contentWin, TRUE);

  wmove(contentWin, 4, 2);
  wrefresh(contentWin);

  wtimeout(contentWin, 100);
  while ((ch = wgetch(contentWin)) != '\n') {
    if (ch == ERR) {
      if (onIdle) onIdle();
      continue;
    }

    if (ch == KEY_RESIZE) {
#ifdef _WIN32
      FullScreenPanel::handleResize();
#else
      wclear(win);
      FullScreenPanel::show();
#endif
      wmove(contentWin, 4, 2 + (int) apiKey.length());
      wrefresh(contentWin);
      continue;
    }

    if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
      if (!apiKey.empty()) {
        apiKey.pop_back();
        int y, x;
        getyx(contentWin, y, x);
        (void)y;
        if (x > 2) {
          mvwaddch(contentWin, y, x - 1, ' ');
          wmove(contentWin, y, x - 1);
        }
      }
    } else if (isprint(ch)) {
      int y, x;
      getyx(contentWin, y, x);
      (void)y;
      if (x < getmaxx(contentWin) - 2 && apiKey.length() < 127) {
        apiKey += (char)ch;
        waddch(contentWin, ch);
      }
    }
    wrefresh(contentWin);
  }

  curs_set(0);
}

std::string PantryConnectPanel::getEnteredKey() const { return apiKey; }
