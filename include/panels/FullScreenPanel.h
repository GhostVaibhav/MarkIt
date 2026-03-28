#pragma once
#include <curses.h>

class FullScreenPanel {
public:
    explicit FullScreenPanel();
    virtual ~FullScreenPanel() = default;

    void show();
    virtual void render() = 0;
    
    void handleResize();
    bool checkSize();

protected:
    WINDOW* win;
    void renderSizeWarning();
};
