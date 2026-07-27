#pragma once
#ifdef MOUSE_MOVED
#undef MOUSE_MOVED
#endif
#include <curses.h>

#include "AddTodoPanel.h"
#include "LoadingPanel.h"
#include "LoginPanel.h"
#include "MainMenuPanel.h"
#include "MenuPanel.h"
#include "PantryConnectPanel.h"
#include "TodoDetailPanel.h"
#include "WelcomePanel.h"

class MainUI {
 private:
  WINDOW* win;

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
};
