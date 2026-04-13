#pragma once
#include <curses.h>

#include <string>
#include <vector>

#include "DimensionConfig.h"
#include "StatsPanel.h"

class FullScreenPanel {
 public:
  explicit FullScreenPanel();
  virtual ~FullScreenPanel();

  void show();
  virtual void render() = 0;

  void handleResize();
  bool checkSize();
  void clearKeyBar();

  virtual int getMinWidth() const { return Dimensions::DefaultMinWidth; }
  virtual int getMinHeight() const { return Dimensions::DefaultMinHeight; }

  void setStats(int total, int completed) {
    if (statsPanel) statsPanel->setStats(total, completed);
  }
  
  void setSyncStatus(int push, int pull) {
    if (statsPanel) statsPanel->setSyncStatus(push, pull);
  }

 protected:
  WINDOW* win;
  WINDOW* bottomBar = nullptr;
  StatsPanel* statsPanel = nullptr;
  void renderSizeWarning();
  void refreshKeyBar(
      const std::vector<std::pair<std::string, std::string>>& keys);
};
