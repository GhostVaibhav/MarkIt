#pragma once
#include <string>

#include "Panel.h"

class StatsPanel : public Panel {
 public:
  StatsPanel(WINDOW* win, int x, int y);
  void render() override;

  void setStats(int total, int completed);
  void setSyncStatus(int pendingPushes, int pendingPulls);

 private:
  int totalTodos = 0;
  int completedTodos = 0;
  int pendingPush = 0;
  int pendingPull = 0;
};
