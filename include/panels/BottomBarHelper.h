#pragma once
#include <curses.h>
#include <vector>
#include <string>

inline WINDOW* drawBottomBar(WINDOW* oldBar, const std::vector<std::pair<std::string, std::string>>& keys) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    if (oldBar) {
        wclear(oldBar);
        wrefresh(oldBar);
        delwin(oldBar);
    }
    
    WINDOW* bar = newwin(1, max_x, max_y - 1, 0);
    wclear(bar);
    
    int currentX = 0;
    for (const auto& k : keys) {
        if (has_colors()) wattron(bar, COLOR_PAIR(6));
        mvwprintw(bar, 0, currentX, " %s ", k.first.c_str());
        currentX += k.first.length() + 2;
        
        if (has_colors()) {
            wattroff(bar, COLOR_PAIR(6));
            wattron(bar, COLOR_PAIR(7));
        } else {
            wattron(bar, A_REVERSE);
        }
        
        mvwprintw(bar, 0, currentX, " %s ", k.second.c_str());
        currentX += k.second.length() + 2;
        
        if (has_colors()) wattroff(bar, COLOR_PAIR(7));
        else wattroff(bar, A_REVERSE);
        
        currentX += 1;
    }
    wrefresh(bar);
    return bar;
}
