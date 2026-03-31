#include "WelcomePanel.h"

WelcomePanel::WelcomePanel(WINDOW* w) : FullScreenPanel(), logoPanel(w, 0, 0) {}

void WelcomePanel::setUsername(const std::string& username) {
  curUser = username;
}
void WelcomePanel::setCode(int c) { code = c; }
int WelcomePanel::getCode() const { return code; }

void WelcomePanel::render() {
  int max_y, max_x;
  getmaxyx(win, max_y, max_x);

  int part = (max_x - 36) / 2;
  int half = ((max_y - 4) / 2) - 1;

  logoPanel.setWindow(win);
  logoPanel.setPosition(half, part);
  logoPanel.render();

  if (code == 1) {
    std::string text = "Welcome, " + curUser;
    mvwprintw(win, max_y - 2, (max_x - 10 - curUser.size()) / 2, "%s",
              text.c_str());
  } else if (code == 2) {
    std::string text = "Welcome back, " + curUser;
    mvwprintw(win, max_y - 2, (max_x - 15 - curUser.size()) / 2, "%s",
              text.c_str());
  }
  wrefresh(win);
  refreshKeyBar({{"Enter", "Continue"}, {"q/Q/^C", "Exit"}});
}

bool WelcomePanel::waitForContinue() {
  keypad(win, TRUE);
  int ch = wgetch(win);
  if (ch == 'q' || ch == 'Q' || ch == 27 || ch == 3) return false;
  return true;
}
