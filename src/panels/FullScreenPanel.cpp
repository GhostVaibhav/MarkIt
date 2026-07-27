#include "FullScreenPanel.h"

#include "BottomBarHelper.h"

FullScreenPanel::FullScreenPanel() : win(stdscr), bottomBar(nullptr) {
  statsPanel = new StatsPanel(win, 0, 0);
}

FullScreenPanel::~FullScreenPanel() {
  if (bottomBar) delwin(bottomBar);
  if (statsPanel) delete statsPanel;
}

void FullScreenPanel::refreshKeyBar(
    const std::vector<std::pair<std::string, std::string>>& keys) {
  bottomBar = drawBottomBar(bottomBar, keys);
}

void FullScreenPanel::clearKeyBar() {
  if (bottomBar) {
    delwin(bottomBar);
    bottomBar = nullptr;
  }
}

void FullScreenPanel::show() {
  if (!checkSize()) {
    renderSizeWarning();
    return;
  }
  wclear(win);
  wrefresh(win);
  render();
}

void FullScreenPanel::handleResize() {
  resize_term(0, 0);
  clear();
  refresh();
  wclear(win);
  show();
}

bool FullScreenPanel::checkSize() {
  int max_y, max_x;
  getmaxyx(win, max_y, max_x);
  return (max_x >= getMinWidth() && max_y >= getMinHeight());
}

void FullScreenPanel::renderSizeWarning() {
  wclear(win);
  int max_y, max_x;
  getmaxyx(win, max_y, max_x);
  mvwprintw(win, max_y / 2, (max_x - 22) / 2, "Please enlarge terminal");
  wrefresh(win);
}

void FullScreenPanel::renderSyncIndicator(WINDOW* targetWin) {
  if (!targetWin) return;
  if (!manualSyncRunning && !manualSyncResultPending) {
    mvwhline(targetWin, 0, 2, ACS_HLINE, 25);
    wrefresh(targetWin);
    return;
  }
  
  std::string text = "";
  if (manualSyncRunning) {
    const char* spinner[] = {"-", "\\", "|", "/"};
    std::string sp = spinner[(manualSyncFrame / 2) % 4];
    if (manualSyncType == SyncOperation::Push) text = "[ " + sp + " Pushing... ]";
    else if (manualSyncType == SyncOperation::Pull) text = "[ " + sp + " Pulling... ]";
    else if (manualSyncType == SyncOperation::Refresh) text = "[ " + sp + " Refreshing... ]";
  } else if (manualSyncResultPending) {
    if (manualSyncResult == SyncResult::Success ||
        manualSyncResult == SyncResult::AlreadyInSync) {
      text = "[ o Synced ]";
    } else if (manualSyncResult == SyncResult::BucketExpired) {
      text = "[ ! Bucket Expired - Push to Recreate ]";
    } else {
      text = "[ ! Sync Failed ]";
    }
  }
  
  if (!text.empty()) {
    wattron(targetWin, A_BOLD);
    mvwprintw(targetWin, 0, 2, "%s", text.c_str());
    wattroff(targetWin, A_BOLD);
    int textLen = text.length();
    mvwhline(targetWin, 0, 2 + textLen, ACS_HLINE, 25 - textLen);
    wrefresh(targetWin);
  }
}
