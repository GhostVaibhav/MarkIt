#pragma once
#include "FullScreenPanel.h"
#include "LogoPanel.h"
#include <string>

class WelcomePanel : public FullScreenPanel {
public:
    explicit WelcomePanel(WINDOW* win);
    void render() override;
    /** Blocks until a key is read. Returns false if the user chose to quit (q, Esc, Ctrl+C). */
    bool waitForContinue();
    
    void setCode(int code);
    void setUsername(const std::string& username);
    int getCode() const;

private:
    LogoPanel logoPanel;
    int code = 0;
    std::string curUser;
};
