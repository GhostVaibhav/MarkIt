#include "LoginPanel.h"
#define BORDER_M(win) wborder(win, 0, 0, 0, 0, 0, 0, 0, 0)

LoginPanel::LoginPanel(WINDOW* w) 
    : FullScreenPanel(), logoPanel(w, 0, 0), loadingPanel(w) {}

LoginPanel::~LoginPanel() {
    if (title) delwin(title);
    if (userNameWindow) delwin(userNameWindow);
    if (passwordWindow) delwin(passwordWindow);
    if (information) delwin(information);
    if (wrongPassword) delwin(wrongPassword);
}

void LoginPanel::recreateWindows() {
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    if (title) delwin(title);
    if (userNameWindow) delwin(userNameWindow);
    if (passwordWindow) delwin(passwordWindow);
    if (information) delwin(information);
    if (wrongPassword) delwin(wrongPassword);
    
    int part = (max_y - 18) / 4;
    title = newwin(8, max_x, part, 0);
    userNameWindow = newwin(5, max_x - 20, 2 * part + 8, 10);
    passwordWindow = newwin(5, max_x - 20, 3 * part + 13, 10);
    information = newwin(3, max_x, max_y - 3, 0);
    wrongPassword = newwin(3, max_x, 10, 0);
}

void LoginPanel::render() {
    if (isLoading) {
        loadingPanel.render();
        return;
    }
    
    recreateWindows();
    wclear(userNameWindow);
    wclear(passwordWindow);
    wclear(title);
    wclear(information);
    
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(win);
    
    BORDER_M(userNameWindow);
    BORDER_M(passwordWindow);
    
    int part = (getmaxx(title) - 34) / 2;
    int half = 0;
    logoPanel.setWindow(title);
    logoPanel.setPosition(half, part);
    logoPanel.render();
    
    mvwprintw(information, 1, (getmaxx(information) - 26) / 2, "Don't resize this window!");
    mvwprintw(userNameWindow, getmaxy(userNameWindow) / 2, 5, "Username: %s", username.c_str());
    
    if (errorMessage == "Wrong Password") {
        wattron(passwordWindow, COLOR_PAIR(2));
        box(passwordWindow, 0, 0);
        mvwprintw(passwordWindow, getmaxy(passwordWindow) / 2, (getmaxx(passwordWindow) - 14) / 2, "Wrong Password");
        wattroff(passwordWindow, COLOR_PAIR(2));
    } else {
        mvwprintw(passwordWindow, getmaxy(passwordWindow) / 2, (getmaxx(passwordWindow) - 20) / 2, "Enter your password");
    }
    
    if (!errorMessage.empty() && errorMessage != "Wrong Password") {
        wattron(wrongPassword, COLOR_PAIR(2));
        mvwprintw(wrongPassword, 2, (getmaxx(wrongPassword) - errorMessage.size()) / 2, "%s", errorMessage.c_str());
        wattroff(wrongPassword, COLOR_PAIR(2));
        wrefresh(wrongPassword);
    }
    
    wrefresh(title);
    wrefresh(information);
    wrefresh(userNameWindow);
    wrefresh(passwordWindow);
}

void LoginPanel::promptInput() {
    char nameBuf[128] = {0};
    char passBuf[128] = {0};
    
    echo();
    wbkgd(userNameWindow, COLOR_PAIR(1));
    wgetnstr(userNameWindow, nameBuf, 127);
    wbkgd(userNameWindow, COLOR_PAIR(6));
    BORDER_M(userNameWindow);
    wrefresh(userNameWindow);
    
    noecho();
    wbkgd(passwordWindow, COLOR_PAIR(1));
    wgetnstr(passwordWindow, passBuf, 127);
    wbkgd(passwordWindow, COLOR_PAIR(6));
    BORDER_M(passwordWindow);
    wrefresh(passwordWindow);
    
    username = std::string(nameBuf);
    password = std::string(passBuf);
}

std::string LoginPanel::getEnteredUsername() const { return username; }
std::string LoginPanel::getEnteredPassword() const { return password; }
void LoginPanel::clearUsername() { username = ""; password = ""; }
void LoginPanel::reset() { clearUsername(); errorMessage = ""; isLoading = false; }
void LoginPanel::showLoading(const std::string& msg) { loadingPanel.setLoadingText(msg); isLoading = true; render(); }
void LoginPanel::showError(const std::string& error) { errorMessage = error; isLoading = false; render(); }
