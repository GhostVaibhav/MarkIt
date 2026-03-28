#include "PantryConnectPanel.h"
#include <curses.h>

PantryConnectPanel::PantryConnectPanel(WINDOW* w) : FullScreenPanel(), logoPanel(w, 0, 0) {}

PantryConnectPanel::~PantryConnectPanel() {
    if (contentWin) delwin(contentWin);
}

void PantryConnectPanel::recreateWindows() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    if (contentWin) delwin(contentWin);
    contentWin = newwin(6, 60, (max_y - 6) / 2 + 5, (max_x - 60) / 2);
}

void PantryConnectPanel::render() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    wclear(stdscr);
    
    int part = (max_x - 81) / 4;
    int half = (max_y - 10) / 4;
    if (part <= 0) part = 1;
    if (half <= 0) half = 1;
    
    logoPanel.setWindow(stdscr);
    logoPanel.setPosition(half, part);
    logoPanel.render();
    
    recreateWindows();
    box(contentWin, 0, 0);
    mvwprintw(contentWin, 2, 2, "Enter Pantry API Key (or empty to cancel):");
    wrefresh(stdscr);
    wrefresh(contentWin);
}

void PantryConnectPanel::promptInput() {
    char keyBuf[128] = {0};
    echo();
    curs_set(1);
    
    mvwgetnstr(contentWin, 3, 2, keyBuf, 127);
    
    noecho();
    curs_set(0);
    apiKey = std::string(keyBuf);
}

std::string PantryConnectPanel::getEnteredKey() const {
    return apiKey;
}
