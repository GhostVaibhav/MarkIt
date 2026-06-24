#include "TodoDetailPanel.h"
#include "utils/StringUtils.h"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

TodoDetailPanel::TodoDetailPanel() 
    : FullScreenPanel(), 
      logoPanel(win, 0, 0),
      todoUserName(nullptr),
      contentWin(nullptr) {}

TodoDetailPanel::~TodoDetailPanel() {
  if (todoUserName) delwin(todoUserName);
  if (contentWin) delwin(contentWin);
}

void TodoDetailPanel::setTodo(const Todo& todo) {
  currentTodo = todo;
  scrollOffset = 0;   // reset scroll whenever a new todo is shown
}

void TodoDetailPanel::setCredentials(const std::string& username, const std::string& id) {
  curUser = username;
  pantryId = id;
}

void TodoDetailPanel::recreateWindows() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  int safe_w = (max_x > 2) ? max_x - 2 : 1;
  int safe_h_win = (max_y > 13) ? max_y - 13 : 1;

  if (!todoUserName) {
    todoUserName = newwin(10, safe_w, 1, 1);
    contentWin = newwin(safe_h_win, safe_w, 11, 1);
  } else {
    mvwin(todoUserName, 0, 0);
    mvwin(contentWin, 0, 0);

#ifdef _WIN32
    resize_window(todoUserName, 10, safe_w);
    resize_window(contentWin, safe_h_win, safe_w);
#else
    wresize(todoUserName, 10, safe_w);
    wresize(contentWin, safe_h_win, safe_w);
#endif

    mvwin(todoUserName, 1, 1);
    mvwin(contentWin, 11, 1);
  }
}

// ─── buildContentLines() ──────────────────────────────────────────────────────
// Returns every logical display line for the current todo, in order.
// This is used both by render() (to display) and by promptAction() (to clamp scroll).
std::vector<std::string> TodoDetailPanel::buildContentLines(int lineWidth) const {
  if (lineWidth < 1) lineWidth = 1;

  std::vector<std::string> lines;

  // Status line
  std::string statusLine = "Status: ";
  statusLine += currentTodo.isComplete ? "Completed" : "Pending";
  lines.push_back("\x01" + statusLine);   // \x01 marker = colour-coded status line

  lines.push_back("");  // blank spacer

  // Name section
  lines.push_back("\x02Name:");           // \x02 marker = bold label
  auto splitAndWrap = [&](const std::string& s) {
    if (s.empty()) {
      lines.push_back("");
      return;
    }
    size_t start = 0;
    while (start < s.size()) {
      size_t len = 0;
      while (start + len < s.size() && len < (size_t)(lineWidth - 2) && s[start + len] != '\n') {
        len++;
      }
      lines.push_back("  " + s.substr(start, len));
      start += len;
      if (start < s.size() && s[start] == '\n') {
        start++;
        if (start == s.size()) {
          lines.push_back("");
        }
      }
    }
  };

  splitAndWrap(currentTodo.name);

  lines.push_back("");  // blank spacer

  // Description section
  lines.push_back("\x02Description:");   // \x02 marker = bold label
  splitAndWrap(currentTodo.desc);

  return lines;
}

// ─── render() ─────────────────────────────────────────────────────────────────

void TodoDetailPanel::render() {
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

  wclear(todoUserName);
  wclear(contentWin);

  box(todoUserName, 0, 0);
  box(contentWin, 0, 0);

  // ── Header ──
  int part = (getmaxx(todoUserName) - 81) / 4;
  if (part <= 0) part = 2;

  logoPanel.setWindow(todoUserName);
  logoPanel.setPosition(1, part);
  logoPanel.render();
  
  if (statsPanel) {
    statsPanel->setWindow(win);
    statsPanel->render();
  }

  int u_x = getmaxx(todoUserName) - 50;
  if (u_x < part + 45) u_x = part + 45;
  int remainingW = getmaxx(todoUserName) - u_x - 1;
  if (remainingW < 3) remainingW = 3;

  std::string dispUser = "Username: " + curUser;
  mvwprintw(todoUserName, 3, u_x, "%s", StringUtils::truncateString(dispUser, remainingW).c_str());
  if (!pantryId.empty() && pantryId != "None") {
    std::string dispId = "Pantry ID: " + pantryId;
    mvwprintw(todoUserName, 5, u_x, "%s", StringUtils::truncateString(dispId, remainingW).c_str());
  }

  // ── Content with scrolling ──
  int cw_max_y, cw_max_x;
  getmaxyx(contentWin, cw_max_y, cw_max_x);

  // Usable area: rows 1..cw_max_y-2  (inside the box border)
  int visibleRows = cw_max_y - 2;
  int lineWidth   = (cw_max_x > 8) ? cw_max_x - 8 : 1;
  int col         = 5;

  auto allLines = buildContentLines(lineWidth);
  int totalLines = (int)allLines.size();

  // Clamp scrollOffset
  int maxScroll = std::max(0, totalLines - visibleRows);
  if (scrollOffset > maxScroll) scrollOffset = maxScroll;
  if (scrollOffset < 0)         scrollOffset = 0;

  // Show scrollbar inside the border if needed
  if (totalLines > visibleRows) {
    int pillSize = std::max(1, (visibleRows * visibleRows) / totalLines);
    int maxPillStart = visibleRows - pillSize;
    int pillStart = 1 + (maxScroll > 0 ? (scrollOffset * maxPillStart) / maxScroll : 0);
    int pillEnd = pillStart + pillSize - 1;

    for (int r = 1; r <= visibleRows; ++r) {
      if (r >= pillStart && r <= pillEnd) {
        wattron(contentWin, A_REVERSE);
        mvwprintw(contentWin, r, cw_max_x - 2, " ");
        wattroff(contentWin, A_REVERSE);
      } else {
        mvwaddch(contentWin, r, cw_max_x - 2, ACS_VLINE);
      }
    }
  }

  // Render visible slice
  for (int i = 0; i < visibleRows && (scrollOffset + i) < totalLines; ++i) {
    int row = i + 1;   // row 0 and cw_max_y-1 are the box border
    const std::string& raw = allLines[scrollOffset + i];

    if (!raw.empty() && raw[0] == '\x01') {
      // Status line — coloured
      std::string text = raw.substr(1);
      if (currentTodo.isComplete)
        wattron(contentWin, COLOR_PAIR(2));
      else
        wattron(contentWin, COLOR_PAIR(1));

      mvwprintw(contentWin, row, col, "%s", text.c_str());

      if (currentTodo.isComplete)
        wattroff(contentWin, COLOR_PAIR(2));
      else
        wattroff(contentWin, COLOR_PAIR(1));

    } else if (!raw.empty() && raw[0] == '\x02') {
      // Bold label
      wattron(contentWin, A_BOLD);
      mvwprintw(contentWin, row, col, "%s", raw.substr(1).c_str());
      wattroff(contentWin, A_BOLD);

    } else {
      mvwprintw(contentWin, row, col, "%s", raw.c_str());
    }
  }

  FullScreenPanel::refreshKeyBar({
    {"m/M",     "Menu"},
    {"e/E",     "Edit"},
    {"Esc/q/Q", "Back"},
    {"Up/Dn",   "Scroll"},
    {"^C",      "Exit"}
  });

  wrefresh(todoUserName);
  wrefresh(contentWin);
}

// ─── promptAction() ───────────────────────────────────────────────────────────

TodoDetailAction TodoDetailPanel::promptAction() {
  keypad(win, TRUE);
  while (true) {
    int ch = wgetch(win);
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
      if (scrollOffset > 0) {
        --scrollOffset;
        render();
      }
      continue;
    }

    if (ch == KEY_DOWN) {
      // Compute max scroll on the fly
      int cw_max_y, cw_max_x;
      getmaxyx(contentWin, cw_max_y, cw_max_x);
      int lineWidth   = (cw_max_x > 8) ? cw_max_x - 8 : 1;
      int visibleRows = cw_max_y - 2;
      int totalLines  = (int)buildContentLines(lineWidth).size();
      int maxScroll   = std::max(0, totalLines - visibleRows);

      if (scrollOffset < maxScroll) {
        ++scrollOffset;
        render();
      }
      continue;
    }

    if (ch == 3)
      return TodoDetailAction::QuitApp;
    else if (ch == 27 || ch == 'q' || ch == 'Q')
      return TodoDetailAction::Back;
    else if (ch == 'm' || ch == 'M')
      return TodoDetailAction::OpenMenu;
    else if (ch == 'e' || ch == 'E')
      return TodoDetailAction::EditTodo;
  }
}
