#pragma once
#include "Panel.h"

class LogoPanel : public Panel {
public:
    LogoPanel(WINDOW* win, int x, int y);
    void render() override;
};
