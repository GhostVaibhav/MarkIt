#include "MainMenuPanel.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#define BORDER_M(win) wborder(win, 0, 0, 0, 0, 0, 0, 0, 0)

MainMenuPanel::MainMenuPanel(WINDOW* w)
    : FullScreenPanel(),
      loadingPanel(w),
      logoPanel(w, 0, 0),
      statsPanel(w, 0, 0) {}

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
void MainMenuPanel::setStats(int total, int completed) {
  statsPanel.setStats(total, completed);
}
void MainMenuPanel::setSyncStatus(int push, int pull) {
  statsPanel.setSyncStatus(push, pull);
}

std::string MainMenuPanel::convertTimeToString(int epoch) const {
  std::time_t temp = epoch;
  std::tm* t = std::localtime(&temp);
  std::ostringstream ss;
  ss << std::put_time(t, "%d %b %Y, %H:%M:%S");
  return ss.str();
}

std::string truncateString(const std::string& str, int width) {
  if (str.length() > width && width > 3) {
    return str.substr(0, width - 3) + "...";
  }
  return str;
}

void MainMenuPanel::recreateWindows() {
  int max_y, max_x;
  getmaxyx(win, max_y, max_x);
  if (todoUserName) delwin(todoUserName);
  if (todoWindow) delwin(todoWindow);
  if (todoBody) delwin(todoBody);

  todoUserName = newwin(10, max_x - 2, 1, 1);
  todoWindow = newwin(max_y - 13, max_x - 2, 11, 1);
  todoBody = newwin(getmaxy(todoWindow) - 4, getmaxx(todoWindow) - 2,
                    getmaxy(todoUserName) + 4, 2);
}

void MainMenuPanel::render() {
  recreateWindows();
  wclear(todoWindow);
  wclear(todoUserName);
  wclear(todoBody);

  box(todoWindow, 0, 0);
  box(todoUserName, 0, 0);

  int part = (getmaxx(todoUserName) - 81) / 4;
  int half = 1;
  if (part <= 0) part = 1;

  logoPanel.setWindow(todoUserName);
  logoPanel.setPosition(half, part);
  logoPanel.render();

  mvwprintw(todoUserName, 3, 3 * part + 25, "Username: %s", curUser.c_str());
  if (!pantryId.empty() && pantryId != "None") {
    mvwprintw(todoUserName, 5, 3 * part + 25, "Pantry ID: %s",
              pantryId.c_str());
  }

  int tabDiv = (getmaxx(todoWindow) - 2) / 3;
  mvwvline(todoWindow, 1, tabDiv, 0, 1);
  mvwvline(todoWindow, 1, 2 * tabDiv, 0, 1);
  mvwhline(todoWindow, 2, 1, 0, getmaxx(todoWindow) - 2);
  mvwprintw(todoWindow, 1, (tabDiv - 4) / 2, "Name");
  mvwprintw(todoWindow, 1, ((3 * tabDiv - 12) / 2) + 1, "Description");
  mvwprintw(todoWindow, 1, ((5 * tabDiv - 13) / 2) + 2, "Created Time");
  BORDER_M(todoWindow);

  for (int i = 0; i < (int)todosList.size(); i++) {
    if (pointerIndex == i) {
      if (todosList.at(i).isComplete)
        wattron(todoBody, COLOR_PAIR(2));
      else
        wattron(todoBody, COLOR_PAIR(1));

      if (!has_colors()) wattron(todoBody, A_REVERSE);
    }

    int maxNameWidth = tabDiv - 4;
    int maxDescWidth = tabDiv - 4;

    std::string dispName = truncateString(todosList[i].name, maxNameWidth);
    std::string dispDesc = truncateString(todosList[i].desc, maxDescWidth);
    std::string timeStr = convertTimeToString(todosList[i].time);

    mvwprintw(todoBody, i + moveFactor, (tabDiv - (int)dispName.size()) / 2,
              "%s", dispName.c_str());
    mvwprintw(todoBody, i + moveFactor,
              ((3 * tabDiv - (int)dispDesc.size()) / 2) + 1, "%s",
              dispDesc.c_str());
    mvwprintw(todoBody, i + moveFactor,
              ((5 * tabDiv - (int)timeStr.size()) / 2) + 2, "%s",
              timeStr.c_str());

    if (pointerIndex == i) {
      if (todosList[i].isComplete)
        wattroff(todoBody, COLOR_PAIR(2));
      else
        wattroff(todoBody, COLOR_PAIR(1));

      if (!has_colors()) wattroff(todoBody, A_REVERSE);
    }
  }
  BORDER_M(todoWindow);

  statsPanel.setWindow(todoUserName);
  statsPanel.render();

  refreshKeyBar({{"m/M", "Menu"},
                 {"Enter", "Detail"},
                 {"d/D", "Delete"},
                 {"q/Q/^C", "Exit"},
                 {"Up/Dn", "Move"}});

  wrefresh(win);
  wrefresh(todoWindow);
  wrefresh(todoUserName);
  wrefresh(todoBody);
}
