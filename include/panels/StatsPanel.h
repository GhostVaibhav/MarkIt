#pragma once
#include <string>

#include "Panel.h"

class StatsPanel : public Panel {
 public:
  StatsPanel(WINDOW* win, int x, int y);
  ~StatsPanel();
  void render() override;
  void render(WINDOW* targetWin);

  void setStats(size_t total, int completed);
  void setSyncStatus(int pendingPushes, int pendingPulls);

 private:
  size_t totalTodos = 0;
  int completedTodos = 0;
  int pendingPush = 0;
  int pendingPull = 0;
  
  WINDOW* statsWin = nullptr;
};
