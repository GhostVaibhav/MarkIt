#include "MainUI.h"

MainUI::MainUI(WINDOW* w) 
    : win(w), loadingPanel(w), loginPanel(w), welcomePanel(w), mainMenuPanel(w), menuPanel(w), addTodoPanel(), pantryConnectPanel(w) {}

void MainUI::render() {
    welcomePanel.show();
}
