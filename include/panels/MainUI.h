#pragma once
#include <curses.h>
#include "LoadingPanel.h"
#include "LoginPanel.h"
#include "WelcomePanel.h"
#include "MainMenuPanel.h"
#include "MenuPanel.h"
#include "AddTodoPanel.h"
#include "PantryConnectPanel.h"
#include "TodoDetailPanel.h"

class MainUI {
public:
    explicit MainUI(WINDOW* win);
    void render();

    LoadingPanel loadingPanel;
    LoginPanel loginPanel;
    WelcomePanel welcomePanel;
    MainMenuPanel mainMenuPanel;
    MenuPanel menuPanel;
    AddTodoPanel addTodoPanel;
    PantryConnectPanel pantryConnectPanel;
    TodoDetailPanel todoDetailPanel;

private:
    WINDOW* win;
};
