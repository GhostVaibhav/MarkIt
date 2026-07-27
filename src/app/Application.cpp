#include "Application.h"

#include <cstdlib>
#include <filesystem>
#include <thread>

#include <curses.h>

#include <stdexcept>

#include "ColorScheme.h"
#include "Logger.h"
#include "LoginResult.h"
#include "picosha2.h"
#include "files/FileManager.h"
#include "utils/PathUtils.h"

// Expose internal ttytype to match old PDCurses resize behavior if defined
#ifdef _WIN32
// PDCurses curses.h already exposes ttytype
#include <process.h>
#endif

Application::Application()
    : userManager(config.dbFile, config.stateFile),
      todoManager(config.dbFile) {}

Application::~Application() {
  updateService.stop();
  bgSyncService.stop();
  if (ui) delete ui;
  endwin();
}

void Application::initCurses() {
#ifdef _WIN32
  ttytype[0] = 25;
  ttytype[1] = static_cast<char>(255);
  ttytype[2] = 80;
  ttytype[3] = static_cast<char>(255);
#endif
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);

  ColorScheme scheme;
  scheme.apply();
}

void Application::initLogger() {
  Logger::getInstance();
  spdlog::info("Application started");
}

void Application::loadState() {
  if (userManager.loadSession()) {
    auto user = userManager.getCurrentUser();
    if (user) {
      todoManager.setCurrentUser(*user);
    }
  }
}

bool Application::handleLogin() {
  spdlog::info("Application: Initiating login sequence routing");
  if (userManager.getCurrentUser()) {
    spdlog::info("Application: User already active from disk session");
    ui->welcomePanel.setCode(2);
    ui->welcomePanel.setUsername(userManager.getCurrentUser()->name);
    ui->welcomePanel.show();
    if (!ui->welcomePanel.waitForContinue()) return false;
    return true;
  }

  ui->loginPanel.reset();

  while (true) {
    ui->loginPanel.show();
    ui->loginPanel.promptInput();

    std::string uname = ui->loginPanel.getEnteredUsername();
    std::string upass = ui->loginPanel.getEnteredPassword();

    if (uname.empty() || upass.empty()) {
      ui->loginPanel.showError("Fields cannot be empty!");
      continue;
    }

    std::vector<unsigned char> phash(picosha2::k_digest_size);
    picosha2::hash256(upass.begin(), upass.end(), phash.begin(), phash.end());
    std::string hashed_pass =
        picosha2::bytes_to_hex_string(phash.begin(), phash.end());

    User matchedUser;
    bool found = false;
    for (const auto& u : userManager.getAllUsers()) {
      if (u.name == uname) {
        matchedUser = u;
        found = true;
        break;
      }
    }

    if (found) {
      spdlog::info("Application: Matched username '{}'", uname);
      if (matchedUser.password != hashed_pass) {
        spdlog::warn(
            "Application: Authentication failed (wrong password) for user '{}'",
            uname);
        ui->loginPanel.showError("Wrong Password");
        ui->loginPanel.clearUsername();
        continue;
      }
      spdlog::info(
          "Application: Authentication explicitly passed for user '{}'", uname);
    } else {
      spdlog::info("Application: Registering new user '{}'", uname);
      std::string id_input = uname + hashed_pass;
      std::vector<unsigned char> ihash(picosha2::k_digest_size);
      picosha2::hash256(id_input.begin(), id_input.end(), ihash.begin(),
                        ihash.end());

      matchedUser.id =
          picosha2::bytes_to_hex_string(ihash.begin(), ihash.end());
      matchedUser.name = uname;
      matchedUser.password = hashed_pass;
      userManager.addUser(matchedUser);
    }

    userManager.setCurrentUser(matchedUser);
    userManager.saveSession();
    todoManager.setCurrentUser(matchedUser);

    ui->welcomePanel.setCode(found ? (int)LoginResult::ExistingUser
                                   : (int)LoginResult::NewUser);
    ui->welcomePanel.setUsername(matchedUser.name);
    ui->welcomePanel.show();
    if (!ui->welcomePanel.waitForContinue()) return false;
    return true;
  }
}

void Application::syncPush() {
  spdlog::info("Application: UI signaled cloud data sync push");
  if (manualSyncRunning.exchange(true)) return;

  auto user = userManager.getCurrentUser();
  if (!user) { manualSyncRunning = false; return; }

  manualSyncType = SyncOperation::Push;
  manualSyncResultPending = false;

  bool wasEnabled = bgSyncService.isEnabled();

  std::thread([this, user, wasEnabled]() {
    bgSyncService.stop();
    SyncResult result = syncManager->pushData(user->id, user->password,
                                              todoManager.getAllTodos());
    if (result == SyncResult::Success)
      spdlog::info("Application: Sync push orchestration succeeded");
    else
      spdlog::error("Application: Sync push orchestration failed");

    if (wasEnabled && !currentPantryId.empty()) {
      bgSyncService.start(
        syncManager.get(), &todoManager,
        [this]() -> std::string { auto u = userManager.getCurrentUser(); return u ? u->id : ""; },
        [this]() -> std::string { auto u = userManager.getCurrentUser(); return u ? u->password : ""; }
      );
    }

    manualSyncResult = result;
    manualSyncResultTime = std::chrono::steady_clock::now();
    manualSyncResultPending = true;
    manualSyncRunning = false;
    syncUpdatePending = true;
    uiCv.notify_all();
  }).detach();
}

void Application::syncPull() {
  spdlog::info("Application: UI signaled cloud data sync pull");
  if (manualSyncRunning.exchange(true)) return;

  auto user = userManager.getCurrentUser();
  if (!user) { manualSyncRunning = false; return; }

  manualSyncType = SyncOperation::Pull;
  manualSyncResultPending = false;

  bool wasEnabled = bgSyncService.isEnabled();

  std::thread([this, user, wasEnabled]() {
    bgSyncService.stop();
    std::vector<Todo> outTodos = todoManager.getAllTodos();
    SyncResult result = syncManager->pullData(user->id, outTodos);

    if (result == SyncResult::Success) {
      spdlog::info(
          "Application: Sync pull orchestration succeeded, merging {} todos",
          outTodos.size());
      auto allTodos = todoManager.getAllTodos();
      for (const auto& t : allTodos) {
        todoManager.removeTodo(t);
      }
      for (const auto& t : outTodos) {
        todoManager.addTodo(t);
      }
    } else if (result == SyncResult::BucketExpired) {
      spdlog::warn(
          "Application: Sync pull found no remote bucket (expired or missing). "
          "Push your local data to recreate the bucket.");
    }

    if (wasEnabled && !currentPantryId.empty()) {
      bgSyncService.start(
        syncManager.get(), &todoManager,
        [this]() -> std::string { auto u = userManager.getCurrentUser(); return u ? u->id : ""; },
        [this]() -> std::string { auto u = userManager.getCurrentUser(); return u ? u->password : ""; }
      );
    }

    manualSyncResult = result;
    manualSyncResultTime = std::chrono::steady_clock::now();
    manualSyncResultPending = true;
    manualSyncRunning = false;
    syncUpdatePending = true;
    uiCv.notify_all();
  }).detach();
}

void Application::syncRefresh() {
  spdlog::info("Application: UI signaled cloud data sync refresh");
  if (manualSyncRunning.exchange(true)) return;

  auto user = userManager.getCurrentUser();
  if (!user) { manualSyncRunning = false; return; }

  manualSyncType = SyncOperation::Refresh;
  manualSyncResultPending = false;

  bool wasEnabled = bgSyncService.isEnabled();

  std::thread([this, user, wasEnabled]() {
    bgSyncService.stop();
    syncManager->refreshData(user->id, user->password,
                             todoManager.getAllTodos());

    if (wasEnabled && !currentPantryId.empty()) {
      bgSyncService.start(
        syncManager.get(), &todoManager,
        [this]() -> std::string { auto u = userManager.getCurrentUser(); return u ? u->id : ""; },
        [this]() -> std::string { auto u = userManager.getCurrentUser(); return u ? u->password : ""; }
      );
    }

    manualSyncResult = SyncResult::Success;
    manualSyncResultTime = std::chrono::steady_clock::now();
    manualSyncResultPending = true;
    manualSyncRunning = false;
    syncUpdatePending = true;
    uiCv.notify_all();
  }).detach();
}

void Application::connectToPantry() {
#ifdef _WIN32
  system("start https://getpantry.cloud");
#elif __APPLE__
  system("open https://getpantry.cloud");
#else
  system("xdg-open https://getpantry.cloud");
#endif
  ui->pantryConnectPanel.setCredentials(userManager.getCurrentUser()->name, currentPantryId);
  ui->pantryConnectPanel.show();
  ui->pantryConnectPanel.promptInput([this]() { pumpBackgroundEvents(&ui->pantryConnectPanel); });
  std::string key = ui->pantryConnectPanel.getEnteredKey();
  if (!key.empty()) {
    auto user = userManager.getCurrentUser();
    if (user) {
      User u = *user;
      u.pantryId = key;
      userManager.updateUser(u);
      currentPantryId = key;
      {
        std::lock_guard<std::mutex> lock(bgSyncService.getSyncMutex());
        pantryFacade = std::make_unique<PantryFacade>(currentPantryId);
        syncManager = std::make_unique<SyncManager>(*pantryFacade);
        syncManager->addObserver(this);
      }
      pantryFacade->createBucket(user->id);
      bgSyncService.stop();
      bgSyncService.start(
        syncManager.get(), &todoManager,
        [this]() -> std::string { auto u = userManager.getCurrentUser(); return u ? u->id : ""; },
        [this]() -> std::string { auto u = userManager.getCurrentUser(); return u ? u->password : ""; }
      );
    }
  }
}

void Application::resizeEvent() {
#ifdef _WIN32
  ui->mainMenuPanel.handleResize();
#else
  wclear(stdscr);
  ui->mainMenuPanel.show();
#endif
}

bool Application::mainLoop() {
  keypad(stdscr, TRUE);
  int selectedTodo = 0;
  int scrollOffset = 0;
  bool needFullRender = true;
  bool needDataRefresh = true;
  std::vector<Todo> todos;

  while (true) {
    if (needDataRefresh || syncUpdatePending) {
      todos = todoManager.getAllTodos();
      needDataRefresh = false;

      // Bound selection after data change
      if (selectedTodo >= (int)todos.size()) {
        selectedTodo = todos.empty() ? 0 : (int)todos.size() - 1;
      }
    }

    int comp = 0;
    for (const auto& t : todos)
      if (t.isComplete) comp++;

    SyncStatus sync = syncManager->getSyncStatus();

    ui->mainMenuPanel.setCredentials(userManager.getCurrentUser()->name,
                                     currentPantryId);
    ui->mainMenuPanel.setSyncStateData(sync.newLocalIds, sync.modifiedLocalIds);
    ui->mainMenuPanel.setTodos(todos);
    ui->mainMenuPanel.setStats(todos.size(), comp);
    ui->mainMenuPanel.setSyncStatus(sync.pendingPushes, sync.pendingPulls);
    ui->todoDetailPanel.setSyncStatus(sync.pendingPushes, sync.pendingPulls);
    ui->menuPanel.setSyncStatus(sync.pendingPushes, sync.pendingPulls);
    ui->addTodoPanel.setCredentials(userManager.getCurrentUser()->name, currentPantryId);
    ui->addTodoPanel.setSyncStatus(sync.pendingPushes, sync.pendingPulls);
    ui->pantryConnectPanel.setSyncStatus(sync.pendingPushes, sync.pendingPulls);
    ui->mainMenuPanel.setSelectedIndex(selectedTodo);
    ui->mainMenuPanel.setScroll(scrollOffset);

    pumpBackgroundEvents(&ui->mainMenuPanel);

    // Pass update info to key bar
    if (updateService.isUpdateReady()) {
      ui->mainMenuPanel.setUpdateVersion(updateService.getUpdateInfo().latestVersion);
    } else {
      ui->mainMenuPanel.setUpdateVersion("");
    }

    if (needFullRender) {
      ui->mainMenuPanel.show();
      needFullRender = false;
    } else {
      ui->mainMenuPanel.renderSyncStateOnly();
    }

    // Non-blocking input: check for a key, if none, wait on CV
    nodelay(stdscr, TRUE);
    int ch = getch();

    if (ch == ERR) {
      // Check if update service sent a notification
      if (updateNotificationPending.exchange(false)) {
        if (updateService.isUpdateReady()) {
          // Show notification bar
          needFullRender = true;
        }
        continue;
      }

      // Sleep efficiently until a short timeout
      {
        std::unique_lock<std::mutex> lk(uiMtx);
        uiCv.wait_for(lk, std::chrono::milliseconds(100));
      }
      continue;
    }

    // Restore blocking mode for sub-panel prompts
    nodelay(stdscr, FALSE);

    if (ch == 27 || ch == 'q' || ch == 'Q' || ch == 3) {
      return false;
    } else if (ch == KEY_UP) {
      if (selectedTodo > 0) selectedTodo--;
    } else if (ch == KEY_DOWN) {
      if (selectedTodo < (int)todos.size() - 1) selectedTodo++;
    }

    // Keep selectedTodo visible in the scroll window
    {
      int visRows = ui->mainMenuPanel.getVisibleRows();
      if (selectedTodo < scrollOffset) scrollOffset = selectedTodo;
      if (selectedTodo >= scrollOffset + visRows) scrollOffset = selectedTodo - visRows + 1;
      if (scrollOffset < 0) scrollOffset = 0;
    }

    if (ch == KEY_UP || ch == KEY_DOWN) {
      // Only redraw the list, not the entire screen
      ui->mainMenuPanel.setSelectedIndex(selectedTodo);
      ui->mainMenuPanel.setScroll(scrollOffset);
      ui->mainMenuPanel.renderList();
      continue;
    }

    if (ch == '\n') {
      if (!todos.empty() && selectedTodo >= 0 &&
          selectedTodo < (int)todos.size()) {
        ui->todoDetailPanel.setCredentials(userManager.getCurrentUser()->name, currentPantryId);
        bool stayInDetail = true;
        while (stayInDetail) {
          ui->todoDetailPanel.setTodo(todos[selectedTodo]);
          ui->todoDetailPanel.show();
          TodoDetailAction action = ui->todoDetailPanel.promptAction([this]() { pumpBackgroundEvents(&ui->todoDetailPanel); });
          
          if (action == TodoDetailAction::QuitApp) {
            return false;
          } else if (action == TodoDetailAction::Back) {
            stayInDetail = false;
          } else if (action == TodoDetailAction::EditTodo) {
            Todo& current = todos[selectedTodo];
            ui->addTodoPanel.promptInput(current.name, current.desc, [this]() { pumpBackgroundEvents(&ui->addTodoPanel); });
            std::string newName = ui->addTodoPanel.getEnteredName();
            std::string newDesc = ui->addTodoPanel.getEnteredDesc();
            if (!newName.empty() && (newName != current.name || newDesc != current.desc)) {
              current.name = newName;
              current.desc = newDesc;
              todoManager.updateTodo(current);
              syncManager->recomputeData(todoManager.getAllTodos());
            }
          } else if (action == TodoDetailAction::OpenMenu) {
            ui->menuPanel.setCredentials(userManager.getCurrentUser()->name, currentPantryId);
            ui->menuPanel.setMenuOptions({"1. Toggle Todo", "2. Delete", "3. Back"});
            ui->menuPanel.setSelectedIndex(0);
            int choice = ui->menuPanel.promptSelection([this]() { pumpBackgroundEvents(&ui->menuPanel); });
            if (choice == 0) { // Toggle
              todoManager.toggleTodo(todos[selectedTodo]);
              syncManager->recomputeData(todoManager.getAllTodos());
              // Immediately re-fetch so TodoDetailPanel sees the updated state
              todos = todoManager.getAllTodos();
              needDataRefresh = false;
            } else if (choice == 1) { // Delete
              todoManager.removeTodo(todos[selectedTodo]);
              syncManager->recomputeData(todoManager.getAllTodos());
              needDataRefresh = true;
              stayInDetail = false;
            }
          }
        }
        needFullRender = true;
      }
    } else if (ch == 'd' || ch == KEY_DC || ch == KEY_BACKSPACE || ch == '\b') {
      if (!todos.empty() && selectedTodo >= 0 &&
          selectedTodo < (int)todos.size()) {
        todoManager.removeTodo(todos[selectedTodo]);
        syncManager->recomputeData(todoManager.getAllTodos());
        needDataRefresh = true;
      }
      needFullRender = true;
    } else if (ch == 'u' || ch == 'U') {
      // Apply staged update
      if (updateService.isUpdateReady()) {
        launchUpdaterAndExit();
        return false;  // Won't reach here — launchUpdaterAndExit exits
      }
    } else if (ch == 'm' || ch == 'M') {
      bool pantryIdPopulated = userManager.getCurrentUser()->pantryId != "";

      ui->menuPanel.setCredentials(userManager.getCurrentUser()->name,
                                   currentPantryId);
      if (pantryIdPopulated) {
        std::string syncLabel = bgSyncService.isEnabled()
            ? "Disable auto-sync" : "Enable auto-sync";
        
        if (manualSyncRunning.load()) {
          ui->menuPanel.setMenuOptions({"1. Add a todo", 
                                        "2. " + syncLabel, 
                                        "3. Edit Pantry link",
                                        "4. Check for updates", 
                                        "5. Logout"});
        } else {
          ui->menuPanel.setMenuOptions({"1. Add a todo", "2. Push all changes",
                                        "3. Pull from the cloud", "4. Refresh",
                                        "5. " + syncLabel, "6. Edit Pantry link",
                                        "7. Check for updates", "8. Logout"});
        }
      } else {
        ui->menuPanel.setMenuOptions(
            {"1. Add a todo", "2. Connect to Pantry",
             "3. Check for updates", "4. Logout"});
      }
      ui->menuPanel.setSelectedIndex(0);
      int choice = ui->menuPanel.promptSelection([this]() { pumpBackgroundEvents(&ui->menuPanel); });
      std::string choiceStr = ui->menuPanel.getOption(choice);

      if (!pantryIdPopulated) {
        if (choiceStr.find("Add a todo") != std::string::npos) {
          ui->addTodoPanel.promptInput("", "", [this]() { pumpBackgroundEvents(&ui->addTodoPanel); });
          std::string nameStr = ui->addTodoPanel.getEnteredName();
          std::string descStr = ui->addTodoPanel.getEnteredDesc();
          if (!nameStr.empty()) {
            todoManager.addTodo(Todo::create(nameStr, descStr));
            syncManager->recomputeData(todoManager.getAllTodos());
          }
          needDataRefresh = true;
        } else if (choiceStr.find("Connect to Pantry") != std::string::npos) {
          connectToPantry();
          needDataRefresh = true;
        } else if (choiceStr.find("Check for updates") != std::string::npos) {
          ui->loadingPanel.setLoadingText("Checking for updates...");
          ui->loadingPanel.show();
          updateService.triggerCheck();
        } else if (choiceStr.find("Logout") != std::string::npos) {
          bgSyncService.stop();
          userManager.clearSession();
          return true;
        }
      } else {
        if (choiceStr.find("Add a todo") != std::string::npos) {
          ui->addTodoPanel.promptInput("", "", [this]() { pumpBackgroundEvents(&ui->addTodoPanel); });
          std::string nameStr = ui->addTodoPanel.getEnteredName();
          std::string descStr = ui->addTodoPanel.getEnteredDesc();
          if (!nameStr.empty()) {
            todoManager.addTodo(Todo::create(nameStr, descStr));
            syncManager->recomputeData(todoManager.getAllTodos());
          }
          needDataRefresh = true;
        } else if (choiceStr.find("Push all changes") != std::string::npos) {
          syncPush();
          needDataRefresh = true;
        } else if (choiceStr.find("Pull from the cloud") != std::string::npos) {
          syncPull();
          needDataRefresh = true;
        } else if (choiceStr.find("Refresh") != std::string::npos) {
          syncRefresh();
          needDataRefresh = true;
        } else if (choiceStr.find("auto-sync") != std::string::npos) {
          bgSyncService.toggle();
        } else if (choiceStr.find("Edit Pantry link") != std::string::npos) {
          connectToPantry();
        } else if (choiceStr.find("Check for updates") != std::string::npos) {
          ui->loadingPanel.setLoadingText("Checking for updates...");
          ui->loadingPanel.show();
          updateService.triggerCheck();
        } else if (choiceStr.find("Logout") != std::string::npos) {
          bgSyncService.stop();
          userManager.clearSession();
          return true;
        }
      }
      needFullRender = true;
    } else if (ch == KEY_RESIZE) {
      this->resizeEvent();
      needFullRender = false; // resizeEvent already did a full render
      continue;
    }
  }
  return false;
}

int Application::run() {
  initLogger();
  initCurses();

  ui = new MainUI(stdscr);

  // Start background update service
  updateService.addObserver(this);
  updateService.start(config.version);

  while (true) {
    bgSyncService.stop();
    loadState();
    if (!userManager.getCurrentUser()) {
      if (!handleLogin()) break;
    }

    currentPantryId = userManager.getCurrentUser()->pantryId;

    if (currentPantryId.empty()) {
      pantryFacade = std::make_unique<PantryFacade>("");
    } else {
      pantryFacade = std::make_unique<PantryFacade>(currentPantryId);
    }
    syncManager = std::make_unique<SyncManager>(*pantryFacade);
    syncManager->addObserver(this);
    
    // Wire the callback to persist the cache to our SQLite DB
    syncManager->setCacheCallback([this](const std::string& uid, const std::string& data) {
      userManager.saveRemoteCache(uid, data);
    });

    // Load offline cache and immediately populate diff state
    std::string dbCache = userManager.getRemoteCache(userManager.getCurrentUser()->id);
    
    // Seamless migration from legacy JSON file to SQLite DB
    if (dbCache.empty()) {
      std::string legacyPath = PathUtils::getDataPath() + "/" + userManager.getCurrentUser()->id + "_remote.json";
      FileManager fm(legacyPath);
      auto legacyContent = fm.readFile();
      if (legacyContent && !legacyContent->empty()) {
        dbCache = *legacyContent;
        userManager.saveRemoteCache(userManager.getCurrentUser()->id, dbCache);
        fm.deleteFile();
        spdlog::info("Application: Successfully migrated legacy remote cache to SQLite for user '{}'", userManager.getCurrentUser()->name);
      }
    }

    if (!dbCache.empty()) {
      syncManager->setRemoteCache(dbCache);
    }
    syncManager->recomputeData(todoManager.getAllTodos());

    // Start background sync only when connected to Pantry
    if (!currentPantryId.empty()) {
      bgSyncService.start(
        syncManager.get(), &todoManager,
        [this]() -> std::string {
          auto u = userManager.getCurrentUser();
          return u ? u->id : "";
        },
        [this]() -> std::string {
          auto u = userManager.getCurrentUser();
          return u ? u->password : "";
        }
      );
    }

    if (!mainLoop()) break;
  }
  return 0;
}

void Application::onSyncStatusChanged(const SyncStatus& status) {
  // Set the flag and wake the main loop so it can do a partial stats refresh
  (void)status;
  syncUpdatePending = true;
  uiCv.notify_one();
}

void Application::onUpdateStatusChanged(UpdateStatus status,
                                        const std::string& version) {
  (void)version;
  if (status == UpdateStatus::Ready) {
    updateNotificationPending = true;
    uiCv.notify_one();
  }
}

void Application::launchUpdaterAndExit() {
  std::string stagedPath = updateService.getStagedBinaryPath();
  if (stagedPath.empty()) {
    spdlog::error("Application: No staged binary found for update");
    return;
  }

  // Resolve current executable path
  std::string currentExe;
#ifdef _WIN32
  wchar_t path[8192] = {0};
  GetModuleFileNameW(NULL, path, 8192);
  currentExe = std::filesystem::path(path).u8string();
#else
  char result[8192];
  ssize_t count = readlink("/proc/self/exe", result, 8192);
  currentExe = std::string(result, (count > 0) ? count : 0);
#endif

  std::string updaterPath = config.updaterPath;

  spdlog::info("Application: Launching updater: {} --binary {} --staged {}",
               updaterPath, currentExe, stagedPath);

  // Shutdown cleanly
  updateService.stop();
  bgSyncService.stop();
  if (ui) { delete ui; ui = nullptr; }
  endwin();

#ifdef _WIN32
  // On Windows, spawn the updater asynchronously so this process can exit and unlock the binary
  _spawnl(_P_NOWAIT, updaterPath.c_str(), "markit_updater",
          "--binary", currentExe.c_str(),
          "--staged", stagedPath.c_str(),
          "--relaunch", NULL);
  _exit(0);
#else
  // On Unix, exec replaces this process with the updater
  execl(updaterPath.c_str(), updaterPath.c_str(),
        "--binary", currentExe.c_str(),
        "--staged", stagedPath.c_str(),
        "--relaunch", nullptr);

  // If exec fails, log and exit
  spdlog::error("Application: execl failed for updater");
  _exit(1);
#endif
}

void Application::pumpBackgroundEvents(FullScreenPanel* activePanel) {
  bool stateChanged = false;

  if (manualSyncRunning) {
    manualSyncFrame++;
    stateChanged = true;
  }

  if (syncUpdatePending.exchange(false)) {
    SyncStatus sync = syncManager->getSyncStatus();
    ui->mainMenuPanel.setSyncStateData(sync.newLocalIds, sync.modifiedLocalIds);
    ui->mainMenuPanel.setSyncStatus(sync.pendingPushes, sync.pendingPulls);
    ui->todoDetailPanel.setSyncStatus(sync.pendingPushes, sync.pendingPulls);
    ui->menuPanel.setSyncStatus(sync.pendingPushes, sync.pendingPulls);
    ui->addTodoPanel.setSyncStatus(sync.pendingPushes, sync.pendingPulls);
    ui->pantryConnectPanel.setSyncStatus(sync.pendingPushes, sync.pendingPulls);
    stateChanged = true;
  }

  if (manualSyncResultPending) {
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - manualSyncResultTime).count() >= 5) {
      manualSyncResultPending = false;
      manualSyncType = SyncOperation::None;
      stateChanged = true;
    }
  }

  if (stateChanged) {
    ui->mainMenuPanel.setManualSyncState(manualSyncRunning, manualSyncType, static_cast<int>(manualSyncResult.load()), manualSyncResultPending, manualSyncFrame);
    ui->todoDetailPanel.setManualSyncState(manualSyncRunning, manualSyncType, static_cast<int>(manualSyncResult.load()), manualSyncResultPending, manualSyncFrame);
    ui->menuPanel.setManualSyncState(manualSyncRunning, manualSyncType, static_cast<int>(manualSyncResult.load()), manualSyncResultPending, manualSyncFrame);
    ui->addTodoPanel.setManualSyncState(manualSyncRunning, manualSyncType, static_cast<int>(manualSyncResult.load()), manualSyncResultPending, manualSyncFrame);
    ui->pantryConnectPanel.setManualSyncState(manualSyncRunning, manualSyncType, static_cast<int>(manualSyncResult.load()), manualSyncResultPending, manualSyncFrame);

    if (activePanel) {
      if (activePanel == &ui->mainMenuPanel) {
        ui->mainMenuPanel.renderList();
      }
      activePanel->renderSyncStateOnly();
    } else {
      ui->mainMenuPanel.renderList();
      ui->mainMenuPanel.renderSyncStateOnly();
    }
  }
}
