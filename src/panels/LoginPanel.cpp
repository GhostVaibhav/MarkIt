#include "LoginPanel.h"

#define BORDER_M(win) wborder(win, 0, 0, 0, 0, 0, 0, 0, 0)

LoginPanel::LoginPanel(WINDOW* w, std::shared_ptr<I18nProvider> i18n)
    : FullScreenPanel(i18n), logoPanel(w, 0, 0, i18n), loadingPanel(w, i18n) {}

LoginPanel::~LoginPanel() {
  if (title) delwin(title);
  if (userNameWindow) delwin(userNameWindow);
  if (passwordWindow) delwin(passwordWindow);
  if (information) delwin(information);
  if (wrongPassword) delwin(wrongPassword);
}

void LoginPanel::recreateWindows() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  int part = (max_y - 18) / 4;
  if (part <= 0) part = 1;

  int input_width = max_x - 20;
  if (input_width < 10) input_width = 10;

  int safe_max_x = (max_x > 0) ? max_x : 1;

  if (!title) {
    title = newwin(8, safe_max_x, part, 0);
    userNameWindow = newwin(5, input_width, 2 * part + 8, 10);
    passwordWindow = newwin(5, input_width, 3 * part + 13, 10);
    information = newwin(3, safe_max_x, max_y - 3, 0);
    wrongPassword = newwin(3, safe_max_x, 10, 0);
  } else {
    mvwin(title, 0, 0);
    mvwin(userNameWindow, 0, 0);
    mvwin(passwordWindow, 0, 0);
    mvwin(information, 0, 0);
    mvwin(wrongPassword, 0, 0);

#ifdef _WIN32
    resize_window(title, 8, safe_max_x);
    resize_window(userNameWindow, 5, input_width);
    resize_window(passwordWindow, 5, input_width);
    resize_window(information, 3, safe_max_x);
    resize_window(wrongPassword, 3, safe_max_x);
#else
    wresize(title, 8, safe_max_x);
    wresize(userNameWindow, 5, input_width);
    wresize(passwordWindow, 5, input_width);
    wresize(information, 3, safe_max_x);
    wresize(wrongPassword, 3, safe_max_x);
#endif

    mvwin(title, part, 0);
    mvwin(userNameWindow, 2 * part + 8, 10);
    mvwin(passwordWindow, 3 * part + 13, 10);
    mvwin(information, max_y - 3, 0);
    mvwin(wrongPassword, 10, 0);
  }
}

void LoginPanel::render() {
  if (isLoading) {
    FullScreenPanel::clearKeyBar();
    loadingPanel.render();
    return;
  }

  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  if (win && win != stdscr) {
#ifdef _WIN32
    resize_window(win, max_y, max_x);
#else
    wresize(win, max_y, max_x);
#endif
  }

  FullScreenPanel::clearKeyBar();

  recreateWindows();

  wclear(userNameWindow);
  wclear(passwordWindow);
  wclear(title);
  wclear(information);

  wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(win);

  BORDER_M(userNameWindow);
  BORDER_M(passwordWindow);

  int logo_x = (getmaxx(title) - 34) / 2;
  if (logo_x < 0) logo_x = 0;
  logoPanel.setWindow(title);
  logoPanel.setPosition(0, logo_x);
  logoPanel.render();

  int u_x = 5;
  if (u_x >= getmaxx(userNameWindow)) u_x = getmaxx(userNameWindow) - 1;
  if (u_x < 0) u_x = 0;
  std::string userPrompt = i18n ? i18n->get("username_label") : "Username: ";
  mvwprintw(userNameWindow, getmaxy(userNameWindow) / 2, u_x, "%s%s",
            userPrompt.c_str(), username.c_str());

  std::string pwPrompt = i18n ? i18n->get("enter_password") : "Enter the password";
  std::string wrongPw = i18n ? i18n->get("wrong_password") : "Wrong Password";

  if (errorMessage == wrongPw || errorMessage == "Wrong Password") {
    wattron(passwordWindow, COLOR_PAIR(2));
    box(passwordWindow, 0, 0);

    int err_x = (getmaxx(passwordWindow) - 14) / 2;
    if (err_x < 0) err_x = 0;
    mvwprintw(passwordWindow, getmaxy(passwordWindow) / 2, err_x,
              "%s", wrongPw.c_str());

    wattroff(passwordWindow, COLOR_PAIR(2));
  } else if (password.empty()) {
    int pw_x = (getmaxx(passwordWindow) - (int) pwPrompt.length()) / 2;
    if (pw_x < 0) pw_x = 0;

    mvwprintw(passwordWindow, getmaxy(passwordWindow) / 2, pw_x, "%s",
              pwPrompt.c_str());
  }

  wrefresh(title);
  wrefresh(information);
  wrefresh(userNameWindow);
  wrefresh(passwordWindow);

  std::string nextStr = i18n ? i18n->get("key_next_field") : "Next field";
  std::string exitStr = i18n ? i18n->get("key_exit") : "Exit";
  FullScreenPanel::refreshKeyBar({{"Enter", nextStr}, {"^C", exitStr}});
}

// local resizeEvent removed

void LoginPanel::captureInput(WINDOW* window, std::string& target, bool masked) {
  int ch;
  keypad(window, TRUE);

  while ((ch = wgetch(window)) != '\n') {
    if (ch == ERR) continue;

    if (ch == KEY_RESIZE) {
#ifdef _WIN32
      FullScreenPanel::handleResize();
#else
      wclear(window);
      FullScreenPanel::show();
#endif
      continue;
    }

    if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
      if (!target.empty()) {
        target.pop_back();
        int y, x;
        getyx(window, y, x);
        if (x > 0) {
          mvwaddch(window, y, x - 1, ' ');
          wmove(window, y, x - 1);
        }
      }
    } else if (isprint(ch)) {
      target += (char)ch;
      if (!masked) waddch(window, ch);
    }

    wrefresh(window);
  }
}

void LoginPanel::promptInput() {
  noecho();

  wbkgd(userNameWindow, COLOR_PAIR(1));
  captureInput(userNameWindow, username, false);
  wbkgd(userNameWindow, COLOR_PAIR(6));
  BORDER_M(userNameWindow);
  wrefresh(userNameWindow);

  wbkgd(passwordWindow, COLOR_PAIR(1));
  captureInput(passwordWindow, password, true);
  wbkgd(passwordWindow, COLOR_PAIR(6));
  BORDER_M(passwordWindow);
  wrefresh(passwordWindow);
}

std::string LoginPanel::getEnteredUsername() const { return username; }
std::string LoginPanel::getEnteredPassword() const { return password; }
void LoginPanel::clearUsername() {
  username = "";
  password = "";
}
void LoginPanel::reset() {
  clearUsername();
  errorMessage = "";
  isLoading = false;
}
void LoginPanel::showLoading(const std::string& msg) {
  loadingPanel.setLoadingText(msg);
  isLoading = true;
  render();
}
void LoginPanel::showError(const std::string& error) {
  errorMessage = error;
  isLoading = false;
  render();
}
