#include "MenuPanel.h"
#include "utils/StringUtils.h"
#include <algorithm>

// local resizeEvent removed

MenuPanel::MenuPanel(WINDOW* w)
    : FullScreenPanel(), logoPanel(w, 0, 0) {}

MenuPanel::~MenuPanel() {
  if (titleWin) delwin(titleWin);
  if (menuWin) delwin(menuWin);
}

void MenuPanel::setMenuOptions(const std::vector<std::string>& opts) {
  options = opts;
}
void MenuPanel::setSelectedIndex(unsigned int index) { pointerIndex = index; }
void MenuPanel::setCredentials(const std::string& username,
                               const std::string& id) {
  curUser = username;
  pantryId = id;
}

void MenuPanel::recreateWindows() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  // Safe dimensions to prevent negative math crash
  int safe_w = (max_x > 2) ? max_x - 2 : 1;
  int safe_h_title = 10;
  int safe_h_menu = (max_y > 12) ? max_y - 12 : 1;

  // Legacy Initialization & Macro Resizing (Matching LoginPanel)
  if (!titleWin) {
    titleWin = newwin(safe_h_title, safe_w, 1, 1);
    menuWin = newwin(safe_h_menu, safe_w, 11, 1);
  } else {
    // Move to 0,0 temporarily to prevent out-of-bounds assertion during resize
    mvwin(titleWin, 0, 0);
    mvwin(menuWin, 0, 0);

#ifdef _WIN32
    resize_window(titleWin, safe_h_title, safe_w);
    resize_window(menuWin, safe_h_menu, safe_w);
#else
    wresize(titleWin, safe_h_title, safe_w);
    wresize(menuWin, safe_h_menu, safe_w);
#endif

    // Move back to final positions
    mvwin(titleWin, 1, 1);
    mvwin(menuWin, 11, 1);
  }
}

void MenuPanel::render() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  // 1. CRITICAL: Resize the parent background FIRST to prevent PDCurses crash
  if (win && win != stdscr) {
#ifdef _WIN32
    resize_window(win, max_y, max_x);
#else
    wresize(win, max_y, max_x);
#endif
  }

  recreateWindows();

  wclear(titleWin);
  wclear(menuWin);
  box(titleWin, 0, 0);
  box(menuWin, 0, 0);

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

  if (!options.empty()) {
    const int maxSize =
        (int) std::max_element(options.begin(), options.end(),
                         [](const std::string& a, const std::string& b) {
                           return a.size() < b.size();
                         })->size();
    for (int i = 0; i < (int)options.size(); i++) {
      // Calculate coordinates safely
      int opt_y = (getmaxy(menuWin) / 2) + (i - (static_cast<int>(options.size()) / 2));
      int opt_x = (getmaxx(menuWin) / 2) - maxSize;

      // CLAMP COORDINATES: Prevent MSVC Heap Corruption #936
      if (opt_y < 0) opt_y = 0;
      if (opt_x < 0) opt_x = 0;
      if (opt_y >= getmaxy(menuWin)) opt_y = getmaxy(menuWin) - 1;

      if (i == (int)pointerIndex) {
        wattron(menuWin, COLOR_PAIR(1));
      }
      
      int maxW = getmaxx(menuWin) - opt_x - 2;
      if (maxW < 3) maxW = 3;
      mvwprintw(menuWin, opt_y, opt_x, "%s", StringUtils::truncateString(options[i], maxW).c_str());
      
      if (i == (int)pointerIndex) {
        wattroff(menuWin, COLOR_PAIR(1));
      }
    }
  }

  if (statsPanel) {
    statsPanel->setWindow(win);
    statsPanel->render();
  }

  wrefresh(titleWin);
  wrefresh(menuWin);
  
  FullScreenPanel::refreshKeyBar(
      {{"Up/Dn", "Move"}, {"Enter", "Select"}, {"Esc/q", "Close menu"}, {"^C", "Exit"}});
}

void MenuPanel::renderMenuItems() {
  if (!menuWin || options.empty()) return;

  wclear(menuWin);
  box(menuWin, 0, 0);

  const int maxSize =
      (int) std::max_element(options.begin(), options.end(),
                       [](const std::string& a, const std::string& b) {
                         return a.size() < b.size();
                       })->size();
  for (int i = 0; i < (int)options.size(); i++) {
    // Calculate coordinates safely
    int opt_y = (getmaxy(menuWin) / 2) + (i - (static_cast<int>(options.size()) / 2));
    int opt_x = (getmaxx(menuWin) / 2) - maxSize;

    // CLAMP COORDINATES: Prevent MSVC Heap Corruption
    if (opt_y < 0) opt_y = 0;
    if (opt_x < 0) opt_x = 0;
    if (opt_y >= getmaxy(menuWin)) opt_y = getmaxy(menuWin) - 1;

    if (i == (int)pointerIndex) {
      wattron(menuWin, COLOR_PAIR(1));
    }
    
    int maxW = getmaxx(menuWin) - opt_x - 2;
    if (maxW < 3) maxW = 3;
    mvwprintw(menuWin, opt_y, opt_x, "%s", StringUtils::truncateString(options[i], maxW).c_str());
    
    if (i == (int)pointerIndex) {
      wattroff(menuWin, COLOR_PAIR(1));
    }
  }
  
  wrefresh(menuWin);
}

int MenuPanel::promptSelection() {
  keypad(stdscr, true);
  
  // Initial render
  FullScreenPanel::show();
  
  while (true) {
    int ch = getch();
    
    if (ch == ERR) continue;

    if (ch == KEY_RESIZE) {
#ifdef _WIN32
      FullScreenPanel::handleResize();
#else
      wclear(win);
      FullScreenPanel::show();
#endif
      continue;
    }

    if (ch == KEY_UP) {
      if ((int) options.size() > 0) {
        if (pointerIndex > 0)
          pointerIndex--;
        else
          pointerIndex = (int) options.size() - 1;
      }
    } else if (ch == KEY_DOWN) {
      if ((int) options.size() > 0) {
        if (pointerIndex < options.size() - 1)
          pointerIndex++;
        else
          pointerIndex = 0;
      }
    } else if (ch == 27 || ch == '\b' || ch == KEY_BACKSPACE || ch == 'q' ||
               ch == 'Q' || ch == 3) {
      return -1;
    } else if (ch == '\n') {
      if (options.empty()) return -1;
      return (int) pointerIndex;
    }
    renderMenuItems();
  }
}
