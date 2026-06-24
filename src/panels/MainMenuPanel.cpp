#define _CRT_SECURE_NO_WARNINGS
#include "MainMenuPanel.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include "utils/StringUtils.h"
#define BORDER_M(win) wborder(win, 0, 0, 0, 0, 0, 0, 0, 0)

MainMenuPanel::MainMenuPanel(WINDOW* w)
    : FullScreenPanel(),
      loadingPanel(w),
      logoPanel(w, 0, 0),
      todoUserName(nullptr), 
      todoWindow(nullptr),   
      todoBody(nullptr) {}   

MainMenuPanel::~MainMenuPanel() {
  if (todoUserName) delwin(todoUserName);
  if (todoWindow) delwin(todoWindow);
  if (todoBody) delwin(todoBody);
}

void MainMenuPanel::setTodos(const std::vector<Todo>& todos) {
  todosList = todos;
}
void MainMenuPanel::setSelectedIndex(int index) { pointerIndex = index; }
void MainMenuPanel::setScroll(int topOffset) { moveFactor = topOffset; }
void MainMenuPanel::setCredentials(const std::string& username,
                                   const std::string& id) {
  curUser = username;
  pantryId = id;
}
void MainMenuPanel::setUpdateVersion(const std::string& version) {
  updateVersion = version;
}

std::string MainMenuPanel::convertTimeToString(int epoch) const {
  std::time_t temp = epoch;
  std::tm* t = std::localtime(&temp);
  std::ostringstream ss;
  ss << std::put_time(t, "%d %b %Y, %H:%M:%S");
  return ss.str();
}

// Local truncateString removed in favor of utils/StringUtils.h

void MainMenuPanel::recreateWindows() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  // Safe dimensions to prevent negative math crash
  int safe_w = (max_x > 2) ? max_x - 2 : 1;
  int safe_h_win = (max_y > 13) ? max_y - 13 : 1;
  int safe_w_body = (safe_w > 2) ? safe_w - 2 : 1;
  int safe_h_body = (safe_h_win > 4) ? safe_h_win - 4 : 1;

  // Legacy Initialization & Macro Resizing (Matching LoginPanel)
  if (!todoUserName) {
    todoUserName = newwin(10, safe_w, 1, 1);
    todoWindow = newwin(safe_h_win, safe_w, 11, 1);
    todoBody = newwin(safe_h_body, safe_w_body, 14, 2);
  } else {
    // Move to 0,0 temporarily to prevent out-of-bounds assertion during resize
    mvwin(todoUserName, 0, 0);
    mvwin(todoWindow, 0, 0);
    mvwin(todoBody, 0, 0);

#ifdef _WIN32
    resize_window(todoUserName, 10, safe_w);
    resize_window(todoWindow, safe_h_win, safe_w);
    resize_window(todoBody, safe_h_body, safe_w_body);
#else
    wresize(todoUserName, 10, safe_w);
    wresize(todoWindow, safe_h_win, safe_w);
    wresize(todoBody, safe_h_body, safe_w_body);
#endif

    // Move back to final positions
    mvwin(todoUserName, 1, 1);
    mvwin(todoWindow, 11, 1);
    mvwin(todoBody, 14, 2);
  }
}

void MainMenuPanel::render() {
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

  wclear(todoWindow);
  wclear(todoUserName);
  wclear(todoBody);

  box(todoWindow, 0, 0);
  box(todoUserName, 0, 0);

  int part = (getmaxx(todoUserName) - 81) / 4;
  if (part <= 0) part = 2;

  logoPanel.setWindow(todoUserName);
  logoPanel.setPosition(1, part);
  logoPanel.render();

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

  int tabDiv = (getmaxx(todoWindow) - 2) / 3;
  if (tabDiv < 1) tabDiv = 1;

  mvwvline(todoWindow, 1, tabDiv, 0, 1);
  mvwvline(todoWindow, 1, 2 * tabDiv, 0, 1);
  mvwhline(todoWindow, 2, 1, 0, getmaxx(todoWindow) - 2);

  // CLAMP HEADER COORDINATES
  int col1 = (tabDiv - 4) / 2;
  int col2 = ((3 * tabDiv - 12) / 2) + 1;
  int col3 = ((5 * tabDiv - 13) / 2) + 2;
  if (col1 < 0) col1 = 0;
  if (col2 < 0) col2 = 0;
  if (col3 < 0) col3 = 0;

  mvwprintw(todoWindow, 1, col1, "Name");
  mvwprintw(todoWindow, 1, col2, "Description");
  mvwprintw(todoWindow, 1, col3, "Created Time");
  BORDER_M(todoWindow);

  int visibleRows = getmaxy(todoBody);

  for (int i = 0; i < (int)todosList.size(); i++) {
    int row = i - moveFactor;
    if (row < 0 || row >= visibleRows) continue;

    if (pointerIndex == i) {
      if (todosList.at(i).isComplete)
        wattron(todoBody, COLOR_PAIR(2));
      else
        wattron(todoBody, COLOR_PAIR(1));

      if (!has_colors()) wattron(todoBody, A_REVERSE);
    }

    int maxNameWidth = tabDiv - 4;
    int maxDescWidth = tabDiv - 4;
    if (maxNameWidth < 1) maxNameWidth = 1;
    if (maxDescWidth < 1) maxDescWidth = 1;

    std::string dispName = StringUtils::truncateString(todosList[i].name, maxNameWidth);
    std::string dispDesc = StringUtils::truncateString(todosList[i].desc, maxDescWidth);
    std::string timeStr = convertTimeToString(todosList[i].time);

    // CLAMP ITEM COORDINATES
    int x1 = (tabDiv - (int)dispName.size()) / 2;
    int x2 = ((3 * tabDiv - (int)dispDesc.size()) / 2) + 1;
    int x3 = ((5 * tabDiv - (int)timeStr.size()) / 2) + 2;

    if (x1 < 0) x1 = 0;
    if (x2 < 0) x2 = 0;
    if (x3 < 0) x3 = 0;

    mvwprintw(todoBody, row, x1, "%s", dispName.c_str());
    mvwprintw(todoBody, row, x2, "%s", dispDesc.c_str());
    mvwprintw(todoBody, row, x3, "%s", timeStr.c_str());

    if (pointerIndex == i) {
      if (todosList[i].isComplete)
        wattroff(todoBody, COLOR_PAIR(2));
      else
        wattroff(todoBody, COLOR_PAIR(1));

      if (!has_colors()) wattroff(todoBody, A_REVERSE);
    }
  }

  // Draw scrollbar on todoBody
  int totalLines = (int)todosList.size();
  int maxScroll = std::max(0, totalLines - visibleRows);
  if (totalLines > visibleRows) {
    int pillSize = std::max(1, (visibleRows * visibleRows) / totalLines);
    int maxPillStart = visibleRows - pillSize;
    int pillStart = (maxScroll > 0 ? (moveFactor * maxPillStart) / maxScroll : 0);
    int pillEnd = pillStart + pillSize - 1;
    int rightCol = getmaxx(todoBody) - 1;

    for (int r = 0; r < visibleRows; ++r) {
      if (r >= pillStart && r <= pillEnd) {
        wattron(todoBody, A_REVERSE);
        mvwprintw(todoBody, r, rightCol, " ");
        wattroff(todoBody, A_REVERSE);
      } else {
        mvwaddch(todoBody, r, rightCol, ACS_VLINE);
      }
    }
  }

  BORDER_M(todoWindow);

  if (statsPanel) {
    statsPanel->setWindow(win);
    statsPanel->render();
  }

  std::vector<std::pair<std::string, std::string>> keyBarItems = {
      {"m/M", "Menu"},
      {"Enter", "Detail"},
      {"d/D", "Delete"},
      {"q/Q/^C", "Exit"},
      {"Up/Dn", "Move"}};

  if (!updateVersion.empty()) {
    keyBarItems.push_back({"u/U", "Update v" + updateVersion});
  }

  FullScreenPanel::refreshKeyBar(keyBarItems);

  wrefresh(todoUserName);
  wrefresh(todoWindow);
  wrefresh(todoBody);
}

int MainMenuPanel::getVisibleRows() const {
  if (!todoBody) return 1;
  int rows = getmaxy(todoBody);
  return rows > 0 ? rows : 1;
}

void MainMenuPanel::renderList() {
  if (!todoBody) return;

  wclear(todoBody);

  int tabDiv = (getmaxx(todoWindow) - 2) / 3;
  if (tabDiv < 1) tabDiv = 1;
  int visibleRows = getmaxy(todoBody);

  for (int i = 0; i < (int)todosList.size(); i++) {
    int row = i - moveFactor;
    if (row < 0 || row >= visibleRows) continue;

    if (pointerIndex == i) {
      if (todosList.at(i).isComplete)
        wattron(todoBody, COLOR_PAIR(2));
      else
        wattron(todoBody, COLOR_PAIR(1));

      if (!has_colors()) wattron(todoBody, A_REVERSE);
    }

    int maxNameWidth = tabDiv - 4;
    int maxDescWidth = tabDiv - 4;
    if (maxNameWidth < 1) maxNameWidth = 1;
    if (maxDescWidth < 1) maxDescWidth = 1;

    std::string dispName = StringUtils::truncateString(todosList[i].name, maxNameWidth);
    std::string dispDesc = StringUtils::truncateString(todosList[i].desc, maxDescWidth);
    std::string timeStr = convertTimeToString(todosList[i].time);

    int x1 = (tabDiv - (int)dispName.size()) / 2;
    int x2 = ((3 * tabDiv - (int)dispDesc.size()) / 2) + 1;
    int x3 = ((5 * tabDiv - (int)timeStr.size()) / 2) + 2;

    if (x1 < 0) x1 = 0;
    if (x2 < 0) x2 = 0;
    if (x3 < 0) x3 = 0;

    mvwprintw(todoBody, row, x1, "%s", dispName.c_str());
    mvwprintw(todoBody, row, x2, "%s", dispDesc.c_str());
    mvwprintw(todoBody, row, x3, "%s", timeStr.c_str());

    if (pointerIndex == i) {
      if (todosList[i].isComplete)
        wattroff(todoBody, COLOR_PAIR(2));
      else
        wattroff(todoBody, COLOR_PAIR(1));

      if (!has_colors()) wattroff(todoBody, A_REVERSE);
    }
  }

  // Draw scrollbar on todoBody
  int totalLines = (int)todosList.size();
  int maxScroll = std::max(0, totalLines - visibleRows);
  if (totalLines > visibleRows) {
    int pillSize = std::max(1, (visibleRows * visibleRows) / totalLines);
    int maxPillStart = visibleRows - pillSize;
    int pillStart = (maxScroll > 0 ? (moveFactor * maxPillStart) / maxScroll : 0);
    int pillEnd = pillStart + pillSize - 1;
    int rightCol = getmaxx(todoBody) - 1;

    for (int r = 0; r < visibleRows; ++r) {
      if (r >= pillStart && r <= pillEnd) {
        wattron(todoBody, A_REVERSE);
        mvwprintw(todoBody, r, rightCol, " ");
        wattroff(todoBody, A_REVERSE);
      } else {
        mvwaddch(todoBody, r, rightCol, ACS_VLINE);
      }
    }
  }

  wrefresh(todoBody);
}

void MainMenuPanel::renderStats() {
  if (statsPanel) {
    statsPanel->setWindow(win);
    statsPanel->render();
  }
}
