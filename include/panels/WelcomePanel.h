#pragma once
#include "FullScreenPanel.h"
#include "LogoPanel.h"
#include <string>

class WelcomePanel : public FullScreenPanel {
public:
    explicit WelcomePanel(WINDOW* win);
    void render() override;
    
    void setCode(int code);
    void setUsername(const std::string& username);
    int getCode() const;

private:
    LogoPanel logoPanel;
    int code = 0;
    std::string curUser;
};
