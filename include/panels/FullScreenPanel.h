#pragma once
#include <curses.h>

#include <string>
#include <vector>

#include "DimensionConfig.h"
#include "StatsPanel.h"
#include "SyncOperation.h"
#include "SyncStatus.h"

class FullScreenPanel {
 public:
  explicit FullScreenPanel();
  virtual ~FullScreenPanel();

  void show();
  virtual void render() = 0;
  virtual void renderSyncStateOnly() {
    if (win) {
      renderSyncIndicator(win);
    }
  }

  void handleResize();
  bool checkSize();
  void clearKeyBar();

  virtual int getMinWidth() const { return Dimensions::DefaultMinWidth; }
  virtual int getMinHeight() const { return Dimensions::DefaultMinHeight; }

  void setStats(size_t total, int completed) {
    if (statsPanel) statsPanel->setStats(total, completed);
  }
  
  void setSyncStatus(int push, int pull) {
    if (statsPanel) statsPanel->setSyncStatus(push, pull);
  }

  void setManualSyncState(bool running, SyncOperation type, int result, bool resultPending, int frame) {
    manualSyncRunning = running;
    manualSyncType = type;
    manualSyncResult = static_cast<SyncResult>(result);
    manualSyncResultPending = resultPending;
    manualSyncFrame = frame;
  }
  
  void renderSyncIndicator(WINDOW* targetWin);

 protected:
  WINDOW* win;
  WINDOW* bottomBar = nullptr;
  StatsPanel* statsPanel = nullptr;
  void renderSizeWarning();
  void refreshKeyBar(
      const std::vector<std::pair<std::string, std::string>>& keys);

  bool manualSyncRunning = false;
  SyncOperation manualSyncType = SyncOperation::None;
  SyncResult manualSyncResult = SyncResult::Success;
  bool manualSyncResultPending = false;
  int manualSyncFrame = 0;
};
