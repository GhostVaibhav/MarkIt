#include "FullScreenPanel.h"
#include "AppConfig.h"
#include "BottomBarHelper.h"

FullScreenPanel::FullScreenPanel() : win(stdscr), bottomBar(nullptr) {}

FullScreenPanel::~FullScreenPanel() {
    if (bottomBar) delwin(bottomBar);
}

void FullScreenPanel::refreshKeyBar(const std::vector<std::pair<std::string, std::string>>& keys) {
    bottomBar = drawBottomBar(bottomBar, keys);
}

void FullScreenPanel::clearKeyBar() {
    if (bottomBar) {
        delwin(bottomBar);
        bottomBar = nullptr;
    }
}

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
