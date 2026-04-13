#include "Application.h"

#include <curses.h>

#include <stdexcept>

#include "ColorScheme.h"
#include "Logger.h"
#include "LoginResult.h"
#include "picosha2.h"

// Expose internal ttytype to match old PDCurses resize behavior if defined
#ifdef _WIN32
// PDCurses curses.h already exposes ttytype
#endif

Application::Application()
    : userManager(config.dbFile, config.stateFile),
      todoManager(config.dbFile) {}

Application::~Application() {
  bgSyncService.stop();
  if (ui) delete ui;
  endwin();
}

void Application::initCurses() {
#ifdef _WIN32
  ttytype[0] = 25;
  ttytype[1] = 255;
  ttytype[2] = 80;
  ttytype[3] = 255;
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
  ui->loadingPanel.setLoadingText("Pushing to cloud...");
  ui->loadingPanel.show();

  auto user = userManager.getCurrentUser();
  if (!user) return;

  std::lock_guard<std::mutex> lock(bgSyncService.getSyncMutex());
  SyncResult result = syncManager->pushData(user->id, user->password,
                                            todoManager.getAllTodos());
  if (result == SyncResult::Success)
    spdlog::info("Application: Sync push orchestration succeeded");
  else
    spdlog::error("Application: Sync push orchestration failed");
  ui->welcomePanel.setCode(0);
}

void Application::syncPull() {
  spdlog::info("Application: UI signaled cloud data sync pull");
  ui->loadingPanel.setLoadingText("Pulling from cloud...");
  ui->loadingPanel.show();

  auto user = userManager.getCurrentUser();
  if (!user) return;

  std::vector<Todo> outTodos;
  SyncResult result;
  {
    std::lock_guard<std::mutex> lock(bgSyncService.getSyncMutex());
    result = syncManager->pullData(user->id, outTodos);
  }

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
  }
}

bool Application::offlineOptionHandling(int choice) {
  if (choice == 0) {
    ui->addTodoPanel.promptInput();
    std::string nameStr = ui->addTodoPanel.getEnteredName();
    std::string descStr = ui->addTodoPanel.getEnteredDesc();
    if (!nameStr.empty()) {
      todoManager.addTodo(Todo::create(nameStr, descStr));
    }
    return true;
  } else if (choice == 1) {
#ifdef _WIN32
    system("start https://getpantry.cloud");
#elif __APPLE__
    system("open https://getpantry.cloud");
#else
    system("xdg-open https://getpantry.cloud");
#endif
    ui->pantryConnectPanel.setCredentials(userManager.getCurrentUser()->name, currentPantryId);
    ui->pantryConnectPanel.show();
    ui->pantryConnectPanel.promptInput();
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
    return true;
  }

  return false;
}

bool Application::onlineOptionHandling(int choice) {
  if (choice == 0) {
    ui->addTodoPanel.promptInput();
    std::string nameStr = ui->addTodoPanel.getEnteredName();
    std::string descStr = ui->addTodoPanel.getEnteredDesc();
    if (!nameStr.empty()) {
      todoManager.addTodo(Todo::create(nameStr, descStr));
    }
    return true;
  } else if (choice == 1) {
    syncPush();
    return true;
  } else if (choice == 2) {
    syncPull();
    return true;
  } else if (choice == 3) {
    ui->loadingPanel.setLoadingText("Refreshing data...");
    ui->loadingPanel.show();
    auto user = userManager.getCurrentUser();
    if (user) {
      std::lock_guard<std::mutex> lock(bgSyncService.getSyncMutex());
      syncManager->refreshData(user->id, user->password,
                               todoManager.getAllTodos());
    }
    return true;
  } else if (choice == 5) {
#ifdef _WIN32
    system("start https://getpantry.cloud");
#elif __APPLE__
    system("open https://getpantry.cloud");
#else
    system("xdg-open https://getpantry.cloud");
#endif
    ui->pantryConnectPanel.setCredentials(userManager.getCurrentUser()->name, currentPantryId);
    ui->pantryConnectPanel.show();
    ui->pantryConnectPanel.promptInput();
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
    return true;
  }

  return false;
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

  while (true) {
    auto todos = todoManager.getAllTodos();

    int comp = 0;
    for (const auto& t : todos)
      if (t.isComplete) comp++;

    SyncStatus sync;
    {
      std::lock_guard<std::mutex> lock(bgSyncService.getSyncMutex());
      sync = syncManager->getSyncStatus();
    }

    ui->mainMenuPanel.setCredentials(userManager.getCurrentUser()->name,
                                     currentPantryId);
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
    ui->mainMenuPanel.show();

    int ch = getch();
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

    if (ch == '\n') {
      if (!todos.empty() && selectedTodo >= 0 &&
          selectedTodo < (int)todos.size()) {
        ui->todoDetailPanel.setCredentials(userManager.getCurrentUser()->name, currentPantryId);
        
        bool stayInDetail = true;
        while (stayInDetail) {
          ui->todoDetailPanel.setTodo(todos[selectedTodo]);
          ui->todoDetailPanel.show();
          TodoDetailAction action = ui->todoDetailPanel.promptAction();
          
          if (action == TodoDetailAction::QuitApp) {
            return false;
          } else if (action == TodoDetailAction::Back) {
            stayInDetail = false;
          } else if (action == TodoDetailAction::OpenMenu) {
            ui->menuPanel.setCredentials(userManager.getCurrentUser()->name, currentPantryId);
            ui->menuPanel.setMenuOptions({"1. Toggle Todo", "2. Delete", "3. Back"});
            ui->menuPanel.setSelectedIndex(0);
            
            int choice = ui->menuPanel.promptSelection();
            if (choice == 0) { // Toggle
              todoManager.toggleTodo(todos[selectedTodo]);
              {
                std::lock_guard<std::mutex> lock(bgSyncService.getSyncMutex());
                syncManager->recomputeData(todoManager.getAllTodos());
              }
              todos = todoManager.getAllTodos();
            } else if (choice == 1) { // Delete
              todoManager.removeTodo(todos[selectedTodo]);
              {
                std::lock_guard<std::mutex> lock(bgSyncService.getSyncMutex());
                syncManager->recomputeData(todoManager.getAllTodos());
              }
              todos = todoManager.getAllTodos();
              if (selectedTodo >= (int)todos.size() - 1) selectedTodo--;
              if (selectedTodo < 0) selectedTodo = 0;
              stayInDetail = false;
            }
          }
        }
      }
    } else if (ch == 'd' || ch == KEY_DC || ch == KEY_BACKSPACE || ch == '\b') {
      if (!todos.empty() && selectedTodo >= 0 &&
          selectedTodo < (int)todos.size()) {
        todoManager.removeTodo(todos[selectedTodo]);
        if (selectedTodo >= (int)todos.size() - 1) selectedTodo--;
        if (selectedTodo < 0) selectedTodo = 0;
        {
          std::lock_guard<std::mutex> lock(bgSyncService.getSyncMutex());
          syncManager->recomputeData(todoManager.getAllTodos());
        }
      }
    } else if (ch == 'm' || ch == 'M') {
      bool pantryIdPopulated = userManager.getCurrentUser()->pantryId != "";

      ui->menuPanel.setCredentials(userManager.getCurrentUser()->name,
                                   currentPantryId);
      if (pantryIdPopulated) {
        std::string syncLabel = bgSyncService.isEnabled()
            ? "5. Disable auto-sync" : "5. Enable auto-sync";
        ui->menuPanel.setMenuOptions({"1. Add a todo", "2. Push all changes",
                                      "3. Pull from the cloud", "4. Refresh",
                                      syncLabel, "6. Edit Pantry link", "7. Logout"});
      } else {
        ui->menuPanel.setMenuOptions(
            {"1. Add a todo", "2. Connect to Pantry", "3. Logout"});
      }
      ui->menuPanel.setSelectedIndex(0);

      int choice = ui->menuPanel.promptSelection();

      if (!pantryIdPopulated) {
        if (offlineOptionHandling(choice)) {
        } else if (choice == 2) {
          bgSyncService.stop();
          userManager.clearSession();
          return true;
        }
      } else {
        if (onlineOptionHandling(choice)) {
        } else if (choice == 4) {
          // Toggle auto-sync
          bgSyncService.toggle();
        } else if (choice == 5) {
          // Edit Pantry link (was index 4)
          onlineOptionHandling(5);  // reuse existing pantry-edit logic at old index 4
        } else if (choice == 6) {
          bgSyncService.stop();
          userManager.clearSession();
          return true;
        }
      }
    } else if (ch == KEY_RESIZE) {
      this->resizeEvent();
      continue;
    }
  }
  return false;
}

int Application::run() {
  initLogger();
  initCurses();

  ui = new MainUI(stdscr);

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

    // Start background sync only when connected to Pantry
    bgSyncService.stop();
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
