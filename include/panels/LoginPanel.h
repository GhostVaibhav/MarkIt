#pragma once
#include "FullScreenPanel.h"
#include "LogoPanel.h"
#include "LoadingPanel.h"
#include <string>

class LoginPanel : public FullScreenPanel {
public:
    explicit LoginPanel(WINDOW* win);
    ~LoginPanel();
    
    void render() override;
    void promptInput();
    std::string getEnteredUsername() const;
    std::string getEnteredPassword() const;
    void clearUsername();
    void reset();
    
    void showLoading(const std::string& msg);
    void showError(const std::string& error);

private:
    LogoPanel logoPanel;
    LoadingPanel loadingPanel;
    std::string username;
    std::string password;
    std::string errorMessage;
    bool isLoading = false;
    
    WINDOW* title = nullptr;
    WINDOW* userNameWindow = nullptr;
    WINDOW* passwordWindow = nullptr;
    WINDOW* information = nullptr;
    WINDOW* wrongPassword = nullptr;
    void recreateWindows();
};
