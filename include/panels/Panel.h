#pragma once
#include <curses.h>

class Panel {
public:
    Panel(WINDOW* win, int x, int y);
    virtual ~Panel() = default;

    virtual void render() = 0;
    
    void setPosition(int newX, int newY) { x = newX; y = newY; }
    void setWindow(WINDOW* newWin) { win = newWin; }

protected:
    WINDOW* win;
    int x;
    int y;
};
