#include "TodoDetailPanel.h"

TodoDetailPanel::TodoDetailPanel() : FullScreenPanel(), logoPanel(win, 0, 0) {}

void TodoDetailPanel::setTodo(const Todo& todo) { currentTodo = todo; }

void TodoDetailPanel::render() {
  int max_y, max_x;
  getmaxyx(win, max_y, max_x);

  wclear(win);
  box(win, 0, 0);

  int logoWidth = 81;
  logoPanel.setWindow(win);
  int logoX = (max_x - logoWidth) / 2;
  if (logoX < 1) logoX = 1;
  logoPanel.setPosition(2, logoX);
  logoPanel.render();

  int yOffset = 11;

  if (currentTodo.isComplete)
    wattron(win, COLOR_PAIR(2));
  else
    wattron(win, COLOR_PAIR(1));

  mvwprintw(win, yOffset, 5, "Status: %s",
            currentTodo.isComplete ? "Completed" : "Pending");

  if (currentTodo.isComplete)
    wattroff(win, COLOR_PAIR(2));
  else
    wattroff(win, COLOR_PAIR(1));

  yOffset += 2;
  wattron(win, A_BOLD);
  mvwprintw(win, yOffset++, 5, "Name:");
  wattroff(win, A_BOLD);

  size_t nameIndex = 0;
  while (nameIndex < currentTodo.name.size() && yOffset < max_y - 12) {
    std::string line = currentTodo.name.substr(nameIndex, max_x - 10);
    mvwprintw(win, yOffset++, 7, "%s", line.c_str());
    nameIndex += max_x - 10;
  }

  yOffset++;
  wattron(win, A_BOLD);
  mvwprintw(win, yOffset++, 5, "Description:");
  wattroff(win, A_BOLD);

  size_t charIndex = 0;
  while (charIndex < currentTodo.desc.size() && yOffset < max_y - 8) {
    std::string line = currentTodo.desc.substr(charIndex, max_x - 10);
    mvwprintw(win, yOffset++, 7, "%s", line.c_str());
    charIndex += max_x - 10;
  }

  wrefresh(win);
  refreshKeyBar({{"Up/Dn", "Move"},
                 {"Enter", "Choose"},
                 {"Esc/q/Q", "Back"},
                 {"^C", "Exit"}});
}

TodoDetailAction TodoDetailPanel::promptAction() {
  int max_y, max_x;
  getmaxyx(win, max_y, max_x);

  int selected = 0;
  std::vector<std::string> options = {"1. Toggle Todo", "2. Delete", "3. Back"};

  keypad(win, TRUE);
  while (true) {
    int startY = max_y - 5;
    for (int i = 0; i < (int)options.size(); i++) {
      if (i == selected) wattron(win, A_REVERSE);
      mvwprintw(win, startY + i, max_x / 2 - 10, "%s", options[i].c_str());
      if (i == selected) wattroff(win, A_REVERSE);
    }
    wrefresh(win);

    int ch = wgetch(win);
    if (ch == KEY_UP && selected > 0)
      selected--;
    else if (ch == KEY_DOWN && selected < (int)options.size() - 1)
      selected++;
    else if (ch == 3)
      return TodoDetailAction::QuitApp;
    else if (ch == 27 || ch == 'q' || ch == 'Q')
      return TodoDetailAction::Back;
    else if (ch == '\n') {
      if (selected == 0) return TodoDetailAction::Toggle;
      if (selected == 1) return TodoDetailAction::Delete;
      if (selected == 2) return TodoDetailAction::Back;
    }
  }
}
