#include "AddTodoPanel.h"
#include <curses.h>
#include "BottomBarHelper.h"

// Windows PDCurses resize trick
void AddTodoPanel::resizeEvent() {
  resize_term(0, 0);
  clear();
  refresh();
  render();
}

AddTodoPanel::AddTodoPanel() : Panel(stdscr, 0, 0), addWin(nullptr) {}

AddTodoPanel::~AddTodoPanel() {
  if (bottomBar) delwin(bottomBar);
  if (addWin) delwin(addWin);
}

void AddTodoPanel::recreateWindows() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  // Clamp the box so it never exceeds terminal dimensions
  int w = (max_x > 62) ? 60 : ((max_x > 2) ? max_x - 2 : 1);
  int h = (max_y > 12) ? 10 : ((max_y > 2) ? max_y - 2 : 1);

  // Safely center the box
  int y = (max_y - h) / 2;
  int x = (max_x - w) / 2;
  if (y < 0) y = 0;
  if (x < 0) x = 0;

  if (!addWin) {
    addWin = newwin(h, w, y, x);
  } else {
    // Park at 0,0 temporarily to prevent Windows PDCurses out-of-bounds assertion
    mvwin(addWin, 0, 0);

#ifdef _WIN32
    resize_window(addWin, h, w);
#else
    wresize(addWin, h, w);
#endif

    // Move back to final centered position
    mvwin(addWin, y, x);
  }
}

void AddTodoPanel::render() {
  // 1. CRITICAL: Resize the parent background FIRST to prevent PDCurses crash
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);
  if (win && win != stdscr) {
#ifdef _WIN32
    resize_window(win, max_y, max_x);
#else
    wresize(win, max_y, max_x);
#endif
  }

  // Safely wipe background to clear ghosting
  wclear(win);
  wrefresh(win);

  recreateWindows();

  wclear(addWin);
  box(addWin, 0, 0);

  // Clamp text printing so it doesn't crash on extremely squashed screens
  if (getmaxy(addWin) > 3) {
    mvwprintw(addWin, 2, 2, "Enter Todo Name (empty to skip):");
    // Print whatever the user has typed so far safely
    int n_x = 2;
    if (n_x < getmaxx(addWin)) mvwprintw(addWin, 3, n_x, "%s", name.c_str()); 
  }
  
  if (getmaxy(addWin) > 6) {
    mvwprintw(addWin, 5, 2, "Enter Todo Description:");
    // Print whatever the user has typed so far safely
    int d_x = 2;
    if (d_x < getmaxx(addWin)) mvwprintw(addWin, 6, d_x, "%s", desc.c_str());
  }

  wrefresh(addWin);

  if (bottomBar) {
    delwin(bottomBar);
    bottomBar = nullptr;
  }
  bottomBar = drawBottomBar(bottomBar, {{"Enter", "Next field"}, {"^C", "Exit"}});
}

// Custom input loop replacing the blocking mvwgetnstr
std::string AddTodoPanel::captureInput(bool isNameField) {
  std::string input = isNameField ? name : desc;
  int ch;
  keypad(addWin, TRUE);

  int start_y = isNameField ? 3 : 6;
  if (start_y >= getmaxy(addWin)) start_y = getmaxy(addWin) - 1;

  // Set initial cursor
  int start_x = 2 + input.length();
  if (start_x >= getmaxx(addWin)) start_x = getmaxx(addWin) - 1;
  
  wmove(addWin, start_y, start_x);
  wrefresh(addWin);

  while ((ch = wgetch(addWin)) != '\n') {
    if (ch == ERR) continue;

    if (ch == KEY_RESIZE) {
      // Sync the state before render so the text redraws correctly
      if (isNameField) name = input; 
      else desc = input;

      this->resizeEvent();

      // Recalculate Y and X in case the screen shrunk too much
      start_y = isNameField ? 3 : 6;
      if (start_y >= getmaxy(addWin)) start_y = getmaxy(addWin) - 1;

      // Safely clamp cursor X so it doesn't crash on the right edge
      int cur_x = 2 + input.length();
      if (cur_x >= getmaxx(addWin)) cur_x = getmaxx(addWin) - 1;

      wmove(addWin, start_y, cur_x);
      wrefresh(addWin);
      continue;
    }

    if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
      if (!input.empty()) {
        input.pop_back();
        int y, x;
        getyx(addWin, y, x);
        if (x > 2) { // Protect the border and margin
          mvwaddch(addWin, y, x - 1, ' ');
          wmove(addWin, y, x - 1);
        }
      }
    } else if (isprint(ch)) {
      int y, x;
      getyx(addWin, y, x);
      // Ensure we don't type over the right border
      if (x < getmaxx(addWin) - 2) { 
        input += (char)ch;
        waddch(addWin, ch);
      }
    }
    wrefresh(addWin);
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

  // Capture Name safely
  name = captureInput(true);

  // Capture Description safely
  desc = captureInput(false);

  curs_set(0);

  // Clean up floating windows
  if (addWin) {
    delwin(addWin);
    addWin = nullptr;
  }
  if (bottomBar) {
    delwin(bottomBar);
    bottomBar = nullptr;
  }
  
  wclear(win);
  wrefresh(win);
}

std::string AddTodoPanel::getEnteredName() const { return name; }
std::string AddTodoPanel::getEnteredDesc() const { return desc; }
