#define _CRT_SECURE_NO_WARNINGS
#include "MainMenuPanel.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include "utils/StringUtils.h"
#define BORDER_M(win) wborder(win, 0, 0, 0, 0, 0, 0, 0, 0)

MainMenuPanel::MainMenuPanel(WINDOW* w, std::shared_ptr<I18nProvider> i18n)
    : FullScreenPanel(i18n),
      loadingPanel(w, i18n),
      logoPanel(w, 0, 0, i18n),
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

void MainMenuPanel::setSyncStateData(const std::unordered_set<std::string>& newIds,
                                     const std::unordered_set<std::string>& modifiedIds) {
  syncNewIds = newIds;
  syncModifiedIds = modifiedIds;
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

  // --- MANUAL SYNC INDICATOR ---
  FullScreenPanel::renderSyncIndicator(todoUserName);

  int part = (getmaxx(todoUserName) - 81) / 4;
  if (part <= 0) part = 2;

  logoPanel.setWindow(todoUserName);
  logoPanel.setPosition(1, part);
  logoPanel.render();

  int u_x = getmaxx(todoUserName) - 50;
  if (u_x < part + 45) u_x = part + 45;
  int remainingW = getmaxx(todoUserName) - u_x - 1;
  if (remainingW < 3) remainingW = 3;

  std::string userLabel = i18n ? i18n->get("username_label") : "Username: ";
  std::string dispUser = userLabel + curUser;
  mvwprintw(todoUserName, 3, u_x, "%s", StringUtils::truncateString(dispUser, remainingW).c_str());
  if (!pantryId.empty() && pantryId != "None") {
    std::string idLabel = i18n ? i18n->get("pantry_id_label") : "Pantry ID: ";
    std::string dispId = idLabel + pantryId;
    mvwprintw(todoUserName, 5, u_x, "%s", StringUtils::truncateString(dispId, remainingW).c_str());
  }

  bool hasPantry = !pantryId.empty() && pantryId != "None";
  int symWidth = hasPantry ? 5 : 0;
  int remainingWidthList = getmaxx(todoWindow) - 2 - symWidth;
  int tabDiv = remainingWidthList / 3;
  if (tabDiv < 1) tabDiv = 1;

  if (hasPantry) {
    mvwvline(todoWindow, 1, symWidth, 0, 1);
  }
  mvwvline(todoWindow, 1, symWidth + tabDiv, 0, 1);
  mvwvline(todoWindow, 1, symWidth + 2 * tabDiv, 0, 1);
  mvwhline(todoWindow, 2, 1, 0, getmaxx(todoWindow) - 2);

  // CLAMP HEADER COORDINATES
  std::string stLabel = i18n ? i18n->get("header_status") : "St";
  std::string nameLabel = i18n ? i18n->get("header_name") : "Name";
  std::string descLabel = i18n ? i18n->get("header_desc") : "Description";
  std::string timeLabel = i18n ? i18n->get("header_time") : "Created Time";

  if (hasPantry) {
    mvwprintw(todoWindow, 1, 2, "%s", stLabel.c_str());
  }
  int col1 = symWidth + (tabDiv - (int)nameLabel.length()) / 2;
  int col2 = symWidth + tabDiv + (tabDiv - (int)descLabel.length()) / 2;
  int col3 = symWidth + 2 * tabDiv + (tabDiv - (int)timeLabel.length()) / 2;
  if (col1 < symWidth + 1) col1 = symWidth + 1;
  if (col2 < symWidth + tabDiv + 1) col2 = symWidth + tabDiv + 1;
  if (col3 < symWidth + 2 * tabDiv + 1) col3 = symWidth + 2 * tabDiv + 1;

  mvwprintw(todoWindow, 1, col1, "%s", nameLabel.c_str());
  mvwprintw(todoWindow, 1, col2, "%s", descLabel.c_str());
  mvwprintw(todoWindow, 1, col3, "%s", timeLabel.c_str());
  BORDER_M(todoWindow);

  int visibleRows = getmaxy(todoBody);

  if (hasPantry) {
    mvwvline(todoBody, 0, symWidth - 1, 0, visibleRows);
  }
  mvwvline(todoBody, 0, symWidth + tabDiv - 1, 0, visibleRows);
  mvwvline(todoBody, 0, symWidth + 2 * tabDiv - 1, 0, visibleRows);

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

    std::string cleanName = todosList[i].name;
    std::replace(cleanName.begin(), cleanName.end(), '\n', ' ');
    std::replace(cleanName.begin(), cleanName.end(), '\r', ' ');
    std::string cleanDesc = todosList[i].desc;
    std::replace(cleanDesc.begin(), cleanDesc.end(), '\n', ' ');
    std::replace(cleanDesc.begin(), cleanDesc.end(), '\r', ' ');

    std::string prefix = "";
    if (hasPantry) {
      if (syncNewIds.count(todosList[i].id)) {
        prefix = "+";
      } else if (syncModifiedIds.count(todosList[i].id)) {
        prefix = "*";
      }
    }

    std::string dispName = StringUtils::truncateString(cleanName, maxNameWidth);
    std::string dispDesc = StringUtils::truncateString(cleanDesc, maxDescWidth);
    std::string timeStr = convertTimeToString(todosList[i].time);

    // CLAMP ITEM COORDINATES
    int x1 = symWidth + (tabDiv - (int)dispName.size()) / 2;
    int x2 = symWidth + tabDiv + (tabDiv - (int)dispDesc.size()) / 2 + 1;
    int x3 = symWidth + 2 * tabDiv + (tabDiv - (int)timeStr.size()) / 2 + 2;

    if (x1 < symWidth + 1) x1 = symWidth + 1;
    if (x2 < symWidth + tabDiv + 1) x2 = symWidth + tabDiv + 1;
    if (x3 < symWidth + 2 * tabDiv + 1) x3 = symWidth + 2 * tabDiv + 1;

    if (hasPantry) {
      mvwprintw(todoBody, row, 1, "%s", prefix.c_str());
    }
    mvwprintw(todoBody, row, x1 - 1, "%s", dispName.c_str());
    mvwprintw(todoBody, row, x2 - 1, "%s", dispDesc.c_str());
    mvwprintw(todoBody, row, x3 - 1, "%s", timeStr.c_str());

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
    statsPanel->render(todoUserName);
  }

  std::vector<std::pair<std::string, std::string>> keyBarItems = {
      {"m/M", i18n ? i18n->get("key_menu") : "Menu"},
      {"Enter", i18n ? i18n->get("key_detail") : "Detail"},
      {"d/D", i18n ? i18n->get("key_delete") : "Delete"},
      {"q/Q/^C", i18n ? i18n->get("key_exit") : "Exit"},
      {"Up/Dn", i18n ? i18n->get("key_move") : "Move"}};

  if (!updateVersion.empty()) {
    std::string updateStr = i18n ? i18n->get("key_update") : "Update v";
    keyBarItems.push_back({"u/U", updateStr + updateVersion});
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

void MainMenuPanel::renderSyncStateOnly() {
  if (!todoUserName) return;
  FullScreenPanel::renderSyncIndicator(todoUserName);
  if (statsPanel) {
    statsPanel->render(todoUserName);
  }
  wrefresh(todoUserName);
}

void MainMenuPanel::renderList() {
  if (!todoBody) return;

  wclear(todoBody);

  bool hasPantry = !pantryId.empty() && pantryId != "None";
  int symWidth = hasPantry ? 5 : 0;
  int remainingW = getmaxx(todoWindow) - 2 - symWidth;
  int tabDiv = remainingW / 3;
  if (tabDiv < 1) tabDiv = 1;
  int visibleRows = getmaxy(todoBody);

  if (hasPantry) {
    mvwvline(todoBody, 0, symWidth - 1, 0, visibleRows);
  }
  mvwvline(todoBody, 0, symWidth + tabDiv - 1, 0, visibleRows);
  mvwvline(todoBody, 0, symWidth + 2 * tabDiv - 1, 0, visibleRows);

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

    std::string cleanName = todosList[i].name;
    std::replace(cleanName.begin(), cleanName.end(), '\n', ' ');
    std::replace(cleanName.begin(), cleanName.end(), '\r', ' ');
    std::string cleanDesc = todosList[i].desc;
    std::replace(cleanDesc.begin(), cleanDesc.end(), '\n', ' ');
    std::replace(cleanDesc.begin(), cleanDesc.end(), '\r', ' ');

    std::string prefix = "";
    if (hasPantry) {
      if (syncNewIds.count(todosList[i].id)) {
        prefix = "+";
      } else if (syncModifiedIds.count(todosList[i].id)) {
        prefix = "*";
      }
    }

    std::string dispName = StringUtils::truncateString(cleanName, maxNameWidth);
    std::string dispDesc = StringUtils::truncateString(cleanDesc, maxDescWidth);
    std::string timeStr = convertTimeToString(todosList[i].time);

    // CLAMP ITEM COORDINATES
    int x1 = symWidth + (tabDiv - (int)dispName.size()) / 2;
    int x2 = symWidth + tabDiv + (tabDiv - (int)dispDesc.size()) / 2 + 1;
    int x3 = symWidth + 2 * tabDiv + (tabDiv - (int)timeStr.size()) / 2 + 2;

    if (x1 < symWidth + 1) x1 = symWidth + 1;
    if (x2 < symWidth + tabDiv + 1) x2 = symWidth + tabDiv + 1;
    if (x3 < symWidth + 2 * tabDiv + 1) x3 = symWidth + 2 * tabDiv + 1;

    if (hasPantry) {
      mvwprintw(todoBody, row, 1, "%s", prefix.c_str());
    }
    mvwprintw(todoBody, row, x1 - 1, "%s", dispName.c_str());
    mvwprintw(todoBody, row, x2 - 1, "%s", dispDesc.c_str());
    mvwprintw(todoBody, row, x3 - 1, "%s", timeStr.c_str());

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
