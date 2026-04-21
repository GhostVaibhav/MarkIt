#include "PantryConnectPanel.h"
#include "utils/StringUtils.h"

#include <curses.h>

PantryConnectPanel::PantryConnectPanel(WINDOW* w)
    : FullScreenPanel(), logoPanel(w, 0, 0) {}

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
  
  if (statsPanel) {
    statsPanel->setWindow(win);
    statsPanel->render();
  }

  int u_x = getmaxx(titleWin) - 50;
  if (u_x < part + 45) u_x = part + 45;
  int remainingW = getmaxx(titleWin) - u_x - 1;
  if (remainingW < 3) remainingW = 3;

  std::string dispUser = "Username: " + curUser;
  mvwprintw(titleWin, 3, u_x, "%s", StringUtils::truncateString(dispUser, remainingW).c_str());
  if (!pantryId.empty() && pantryId != "None") {
    std::string dispId = "Pantry ID: " + pantryId;
    mvwprintw(titleWin, 5, u_x, "%s", StringUtils::truncateString(dispId, remainingW).c_str());
  }

  mvwprintw(contentWin, 2, 2, "Enter Pantry API Key (or empty to cancel):");
  mvwprintw(contentWin, 4, 2, "%s", apiKey.c_str());

  wrefresh(titleWin);
  wrefresh(contentWin);
  FullScreenPanel::refreshKeyBar({{"Enter", "Submit"}, {"empty", "Cancel"}, {"^C", "Exit"}});
}

void PantryConnectPanel::promptInput() {
  noecho();
  curs_set(1);
  apiKey = "";

  int ch;
  keypad(contentWin, TRUE);

  wmove(contentWin, 4, 2);
  wrefresh(contentWin);

  while ((ch = wgetch(contentWin)) != '\n') {
    if (ch == ERR) continue;

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
        if (x > 2) {
          mvwaddch(contentWin, y, x - 1, ' ');
          wmove(contentWin, y, x - 1);
        }
      }
    } else if (isprint(ch)) {
      int y, x;
      getyx(contentWin, y, x);
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
