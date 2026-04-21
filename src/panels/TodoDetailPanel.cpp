#include "TodoDetailPanel.h"
#include "utils/StringUtils.h"

TodoDetailPanel::TodoDetailPanel() 
    : FullScreenPanel(), 
      logoPanel(win, 0, 0),
      todoUserName(nullptr),
      contentWin(nullptr) {}

TodoDetailPanel::~TodoDetailPanel() {
  if (todoUserName) delwin(todoUserName);
  if (contentWin) delwin(contentWin);
}

void TodoDetailPanel::setTodo(const Todo& todo) { currentTodo = todo; }

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

  int part = (getmaxx(todoUserName) - 81) / 4;
  if (part <= 0) part = 2;

  // Render header
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

  // Render detail
  int cw_max_y, cw_max_x;
  getmaxyx(contentWin, cw_max_y, cw_max_x);

  int yOffset = 2; // top margin

  if (currentTodo.isComplete)
    wattron(contentWin, COLOR_PAIR(2));
  else
    wattron(contentWin, COLOR_PAIR(1));

  mvwprintw(contentWin, yOffset, 5, "Status: %s",
            currentTodo.isComplete ? "Completed" : "Pending");

  if (currentTodo.isComplete)
    wattroff(contentWin, COLOR_PAIR(2));
  else
    wattroff(contentWin, COLOR_PAIR(1));

  yOffset += 2;
  wattron(contentWin, A_BOLD);
  mvwprintw(contentWin, yOffset++, 5, "Name:");
  wattroff(contentWin, A_BOLD);

  size_t nameIndex = 0;
  while (nameIndex < currentTodo.name.size() && yOffset < cw_max_y - 2) {
    std::string line = currentTodo.name.substr(nameIndex, cw_max_x - 10);
    mvwprintw(contentWin, yOffset++, 7, "%s", line.c_str());
    nameIndex += cw_max_x - 10;
  }

  yOffset++;
  if (yOffset < cw_max_y - 2) {
    wattron(contentWin, A_BOLD);
    mvwprintw(contentWin, yOffset++, 5, "Description:");
    wattroff(contentWin, A_BOLD);
  }

  size_t charIndex = 0;
  while (charIndex < currentTodo.desc.size() && yOffset < cw_max_y - 2) {
    std::string line = currentTodo.desc.substr(charIndex, cw_max_x - 10);
    mvwprintw(contentWin, yOffset++, 7, "%s", line.c_str());
    charIndex += cw_max_x - 10;
  }

  FullScreenPanel::refreshKeyBar({
    {"m/M", "Menu"},
    {"Esc/q/Q", "Back"},
    {"^C", "Exit"}
  });

  wrefresh(todoUserName);
  wrefresh(contentWin);
}

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

    if (ch == 3)
      return TodoDetailAction::QuitApp;
    else if (ch == 27 || ch == 'q' || ch == 'Q')
      return TodoDetailAction::Back;
    else if (ch == 'm' || ch == 'M')
      return TodoDetailAction::OpenMenu;
  }
}
