#include "AddTodoPanel.h"
#include "utils/StringUtils.h"
#include <curses.h>

AddTodoPanel::AddTodoPanel() 
    : FullScreenPanel(), logoPanel(win, 0, 0), titleWin(nullptr), contentWin(nullptr) {}

AddTodoPanel::~AddTodoPanel() {
  if (titleWin) delwin(titleWin);
  if (contentWin) delwin(contentWin);
}

void AddTodoPanel::setCredentials(const std::string& username, const std::string& id) {
  curUser = username;
  pantryId = id;
}

void AddTodoPanel::recreateWindows() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  int safe_w = (max_x > 2) ? max_x - 2 : 1;
  int safe_h_title = 10;
  int safe_h_menu = (max_y > 12) ? max_y - 12 : 1;

  if (!titleWin) {
    titleWin = newwin(safe_h_title, safe_w, 1, 1);
    contentWin = newwin(safe_h_menu, safe_w, 11, 1);
  } else {
    mvwin(titleWin, 0, 0);
    mvwin(contentWin, 0, 0);

#ifdef _WIN32
    resize_window(titleWin, safe_h_title, safe_w);
    resize_window(contentWin, safe_h_menu, safe_w);
#else
    wresize(titleWin, safe_h_title, safe_w);
    wresize(contentWin, safe_h_menu, safe_w);
#endif

    mvwin(titleWin, 1, 1);
    mvwin(contentWin, 11, 1);
  }
}

void AddTodoPanel::render() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  if (win && win != stdscr) {
#ifdef _WIN32
    resize_window(win, max_y, max_x);
#else
    wresize(win, max_y, max_x);
#endif
  }

  wclear(win);
  wrefresh(win);

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

  // Render input fields
  if (getmaxy(contentWin) > 3) {
    mvwprintw(contentWin, 2, 2, "Enter Todo Name (empty to skip):");
    int n_x = 2;
    if (n_x < getmaxx(contentWin)) mvwprintw(contentWin, 3, n_x, "%s", name.c_str()); 
  }
  
  if (getmaxy(contentWin) > 6) {
    mvwprintw(contentWin, 5, 2, "Enter Todo Description:");
    int d_x = 2;
    if (d_x < getmaxx(contentWin)) mvwprintw(contentWin, 6, d_x, "%s", desc.c_str());
  }

  if (statsPanel) {
    statsPanel->setWindow(win);
    statsPanel->render();
  }

  wrefresh(titleWin);
  wrefresh(contentWin);

  refreshKeyBar({
    {"Enter", "Next field"},
    {"^C", "Exit"}
  });
}

// Custom input loop replacing the blocking mvwgetnstr
std::string AddTodoPanel::captureInput(bool isNameField) {
  std::string input = isNameField ? name : desc;
  int ch;
  keypad(contentWin, TRUE);

  int start_y = isNameField ? 3 : 6;
  if (start_y >= getmaxy(contentWin)) start_y = getmaxy(contentWin) - 1;

  // Set initial cursor
  int start_x = 2 + input.length();
  if (start_x >= getmaxx(contentWin)) start_x = getmaxx(contentWin) - 1;
  
  wmove(contentWin, start_y, start_x);
  wrefresh(contentWin);

  while ((ch = wgetch(contentWin)) != '\n') {
    if (ch == ERR) continue;

    if (ch == KEY_RESIZE) {
      if (isNameField) name = input; 
      else desc = input;

#ifdef _WIN32
      this->handleResize();
#else
      wclear(win);
      this->show();
#endif

      start_y = isNameField ? 3 : 6;
      if (start_y >= getmaxy(contentWin)) start_y = getmaxy(contentWin) - 1;

      int cur_x = 2 + input.length();
      if (cur_x >= getmaxx(contentWin)) cur_x = getmaxx(contentWin) - 1;

      wmove(contentWin, start_y, cur_x);
      wrefresh(contentWin);
      continue;
    }

    if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
      if (!input.empty()) {
        input.pop_back();
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
      if (x < getmaxx(contentWin) - 2) { 
        input += (char)ch;
        waddch(contentWin, ch);
      }
    }
    wrefresh(contentWin);
  }
  
  if (isNameField) name = input; 
  else desc = input;
  
  return input;
}

void AddTodoPanel::promptInput() {
  noecho();
  curs_set(1);

  name = "";
  desc = "";

  render();

  name = captureInput(true);
  desc = captureInput(false);

  curs_set(0);

  wclear(win);
  wrefresh(win);
}

std::string AddTodoPanel::getEnteredName() const { return name; }
std::string AddTodoPanel::getEnteredDesc() const { return desc; }
