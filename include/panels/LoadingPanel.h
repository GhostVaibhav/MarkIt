#pragma once
#include "FullScreenPanel.h"
#include "LogoPanel.h"
#include <string>

class LoadingPanel : public FullScreenPanel {
public:
    explicit LoadingPanel(WINDOW* win);
    void setLoadingText(const std::string& text);
    void render() override;

private:
    LogoPanel logoPanel;
    std::string loadingText = "Loading...";
};
