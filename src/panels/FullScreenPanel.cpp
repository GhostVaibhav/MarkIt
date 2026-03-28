#include "FullScreenPanel.h"
#include "AppConfig.h"

FullScreenPanel::FullScreenPanel() : win(stdscr) {}

void FullScreenPanel::show() {
    if (!checkSize()) {
        renderSizeWarning();
        return;
    }
    wclear(win);
    wrefresh(win);
    render();
}

void FullScreenPanel::handleResize() {
    // Basic terminal resize handler
    show();
}

bool FullScreenPanel::checkSize() {
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    AppConfig config;
    return (max_x >= config.minWidth && max_y >= config.minHeight);
}

void FullScreenPanel::renderSizeWarning() {
    wclear(win);
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    mvwprintw(win, max_y / 2, (max_x - 22) / 2, "Please enlarge terminal");
    wrefresh(win);
}
