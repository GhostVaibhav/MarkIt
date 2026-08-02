#include "WelcomePanel.h"

WelcomePanel::WelcomePanel(WINDOW* w, std::shared_ptr<I18nProvider> i18n) : FullScreenPanel(i18n), logoPanel(w, 0, 0, i18n) {}

void WelcomePanel::setUsername(const std::string& username) {
  curUser = username;
}
void WelcomePanel::setCode(int c) { code = c; }
int WelcomePanel::getCode() const { return code; }

void WelcomePanel::render() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  // 1. Resize parent to prevent PDCurses out-of-bounds assertions
  if (win && win != stdscr) {
#ifdef _WIN32
    resize_window(win, max_y, max_x);
#else
    wresize(win, max_y, max_x);
#endif
  }

  wclear(win); // WelcomePanel actually needs this since it renders directly onto win

  // Clamp logo dimensions safely
  int part = (max_x - 36) / 2;
  if (part <= 0) part = 0; 
  int half = ((max_y - 4) / 2) - 1;
  if (half <= 0) half = 0;
  
  FullScreenPanel::clearKeyBar();

  logoPanel.setWindow(win);
  logoPanel.setPosition(half, part);
  logoPanel.render();

  // Clamp Y-coordinate for text so it never writes at -1 on a squashed screen
  int text_y = max_y - 2;
  if (text_y < 0) text_y = 0;

  if (code == 1) {
    std::string text = (i18n ? i18n->get("welcome_msg") : "Welcome, ") + curUser;
    
    // Perfectly center text and clamp X to prevent MSVC Heap Corruption
    int text_x = (max_x - (int) text.size()) / 2;
    if (text_x < 0) text_x = 0; 
    
    mvwprintw(win, text_y, text_x, "%s", text.c_str());
  } else if (code == 2) {
    std::string text = (i18n ? i18n->get("welcome_back_msg") : "Welcome back, ") + curUser;
    
    int text_x = (max_x - (int) text.size()) / 2;
    if (text_x < 0) text_x = 0;
    
    mvwprintw(win, text_y, text_x, "%s", text.c_str());
  }

  wrefresh(win);

  std::string continueStr = i18n ? i18n->get("key_continue") : "Continue";
  std::string exitStr = i18n ? i18n->get("key_exit") : "Exit";
  FullScreenPanel::refreshKeyBar({{"Enter", continueStr}, {"q/Q/^C", exitStr}});
}

bool WelcomePanel::waitForContinue() {
  keypad(win, TRUE);
  int ch;

  // Initial render before waiting for input
  render();

  while ((ch = wgetch(win)) != '\n') {
    if (ch == 'q' || ch == 'Q' || ch == 27 || ch == 3) return false;
    
    if (ch == ERR) continue;

    if (ch == KEY_RESIZE) {
#ifdef _WIN32
      FullScreenPanel::handleResize();
#else
      wclear(win);
      FullScreenPanel::show();
#endif
    }
  }

  return true;
}
