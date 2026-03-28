#pragma once
#include "AppConfig.h"
#include "KeyFileManager.h"
#include "UserManager.h"
#include "TodoManager.h"
#include "SyncManager.h"
#include "PantryFacade.h"
#include "MainUI.h"
#include <memory>

class Application {
public:
    Application();
    ~Application();
    
    int run();

private:
    void initCurses();
    void initLogger();
    
    void loadState();
    
    bool handleLogin();
    bool mainLoop();
    
    void syncPush();
    void syncPull();

    AppConfig config;
    UserManager userManager;
    TodoManager todoManager;
    MainUI* ui = nullptr;
    
    std::unique_ptr<PantryFacade> pantryFacade;
    std::unique_ptr<SyncManager> syncManager;
    
    std::string currentPantryId;
};
