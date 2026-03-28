#include "MenuPanel.h"
#include <algorithm>

MenuPanel::MenuPanel(WINDOW* w) 
    : FullScreenPanel(), logoPanel(w, 0, 0), statsPanel(w, 0, 0) {}

MenuPanel::~MenuPanel() {
    if (titleWin) delwin(titleWin);
    if (menuWin) delwin(menuWin);
}

void MenuPanel::setMenuOptions(const std::vector<std::string>& opts) { options = opts; }
void MenuPanel::setSelectedIndex(unsigned int index) { pointerIndex = index; }
void MenuPanel::setCredentials(const std::string& username, const std::string& id) { 
    curUser = username; 
    pantryId = id; 
}

void MenuPanel::recreateWindows() {
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    if (titleWin) delwin(titleWin);
    if (menuWin) delwin(menuWin);
    titleWin = newwin(10, max_x - 2, 1, 1);
    menuWin = newwin(max_y - 12, max_x - 2, 11, 1);
}

void MenuPanel::render() {
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    recreateWindows();
    
    int part = (getmaxx(titleWin) - 81) / 4;
    int half = 1;
    if (part <= 0) part = 1;
    
    wclear(titleWin);
    wclear(menuWin);
    box(titleWin, 0, 0);
    box(menuWin, 0, 0);
    
    logoPanel.setWindow(titleWin);
    logoPanel.setPosition(half, part);
    logoPanel.render();
    
    mvwprintw(titleWin, 3, 3 * part + 25, "Username: %s", curUser.c_str());
    if (!pantryId.empty() && pantryId != "None") {
        mvwprintw(titleWin, 5, 3 * part + 25, "Pantry ID: %s", pantryId.c_str());
    }
    
    if (!options.empty()) {
        const int maxSize = std::max_element(options.begin(), options.end(), 
            [](const std::string& a, const std::string& b) { return a.size() < b.size(); })->size();
        for (int i = 0; i < (int)options.size(); i++) {
            if (i != (int)pointerIndex) {
                mvwprintw(menuWin, (getmaxy(menuWin) / 2) + (i - (options.size() / 2)), (getmaxx(menuWin) / 2) - maxSize, "%s", options[i].c_str());
            } else {
                wattron(menuWin, COLOR_PAIR(1));
                mvwprintw(menuWin, (getmaxy(menuWin) / 2) + (i - (options.size() / 2)), (getmaxx(menuWin) / 2) - maxSize, "%s", options[i].c_str());
                wattroff(menuWin, COLOR_PAIR(1));
            }
        }
    }
    
    statsPanel.setWindow(win);
    statsPanel.render();
    
    wrefresh(win);
    
    wrefresh(titleWin);
    wrefresh(menuWin);
    refreshKeyBar({{"Up/Dn", "Move"}, {"Enter", "Select"}, {"Esc/q/^C", "Close menu"}});
}

int MenuPanel::promptSelection() {
    keypad(win, true);
    while (true) {
        render();
        int ch = getch();
        if (ch == KEY_UP) {
            if (pointerIndex > 0) pointerIndex--;
            else pointerIndex = 0;
        } else if (ch == KEY_DOWN) {
            if (pointerIndex < options.size() - 1) pointerIndex++;
        } else if (ch == 27 || ch == '\b' || ch == KEY_BACKSPACE || ch == 'q' || ch == 'Q' || ch == 3) {
            return -1;
        } else if (ch == '\n') {
            return pointerIndex;
        }
    }
}
