#pragma once
#include <curses.h>
#include <string>
#include <utility>
#include <vector>

class FullScreenPanel {
public:
    explicit FullScreenPanel();
    virtual ~FullScreenPanel();

    void show();
    virtual void render() = 0;
    
    void handleResize();
    bool checkSize();
    void clearKeyBar();

protected:
    WINDOW* win;
    WINDOW* bottomBar = nullptr;
    void renderSizeWarning();
    void refreshKeyBar(const std::vector<std::pair<std::string, std::string>>& keys);
};
