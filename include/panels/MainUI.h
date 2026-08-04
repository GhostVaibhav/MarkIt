#pragma once
#ifdef MOUSE_MOVED
#undef MOUSE_MOVED
#endif
#include <curses.h>

#include "AddTodoPanel.h"
#include "i18n/I18nProvider.h"
#include <memory>
#include "LoadingPanel.h"
#include "LoginPanel.h"
#include "MainMenuPanel.h"
#include "MenuPanel.h"
#include "PantryConnectPanel.h"
#include "TodoDetailPanel.h"
#include "WelcomePanel.h"
#include "LanguageSelectionPanel.h"

class MainUI {
 private:
  WINDOW* win;

 public:
  explicit MainUI(WINDOW* win, std::shared_ptr<I18nProvider> i18n = nullptr);
  void render();

  LoadingPanel loadingPanel;
  LoginPanel loginPanel;
  WelcomePanel welcomePanel;
  MainMenuPanel mainMenuPanel;
  MenuPanel menuPanel;
  AddTodoPanel addTodoPanel;
  PantryConnectPanel pantryConnectPanel;
  TodoDetailPanel todoDetailPanel;
  LanguageSelectionPanel languageSelectionPanel;
};
