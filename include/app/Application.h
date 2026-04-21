#pragma once
#include <memory>
#include <atomic>
#include <condition_variable>

#include "AppConfig.h"
#include "BackgroundSyncService.h"
#include "KeyFileManager.h"
#include "PantryFacade.h"
#include "SyncManager.h"
#include "TodoManager.h"
#include "UserManager.h"
#include "MainUI.h"
#include "ISyncObserver.h"

class Application : public ISyncObserver {
 public:
  Application();
  ~Application();

  int run();
  void onSyncStatusChanged(const SyncStatus& status) override;

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
  BackgroundSyncService bgSyncService;

  std::string currentPantryId;

  // Used by background sync to wake the main loop
  std::mutex uiMtx;
  std::condition_variable uiCv;
  std::atomic<bool> syncUpdatePending{false};
};
