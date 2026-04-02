#pragma once
#include <memory>

#include "AppConfig.h"
#include "KeyFileManager.h"
#include "PantryFacade.h"
#include "SyncManager.h"
#include "TodoManager.h"
#include "UserManager.h"
#include "MainUI.h"

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
  bool offlineOptionHandling(int);
  bool onlineOptionHandling(int);

  void resizeEvent();
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
