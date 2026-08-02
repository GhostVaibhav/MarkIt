#include "MainUI.h"

MainUI::MainUI(WINDOW* w, std::shared_ptr<I18nProvider> i18n)
    : win(w),
      loadingPanel(w, i18n),
      loginPanel(w, i18n),
      welcomePanel(w, i18n),
      mainMenuPanel(w, i18n),
      menuPanel(w, i18n),
      addTodoPanel(i18n),
      pantryConnectPanel(w, i18n),
      todoDetailPanel(i18n) {}

void MainUI::render() { welcomePanel.show(); }
