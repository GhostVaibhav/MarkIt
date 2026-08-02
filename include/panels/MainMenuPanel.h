#pragma once
#include <string>
#include <unordered_set>
#include <vector>

#include "DimensionConfig.h"
#include "FullScreenPanel.h"
#include "LoadingPanel.h"
#include "LogoPanel.h"
#include "Todo.h"

class MainMenuPanel : public FullScreenPanel {
 public:
  explicit MainMenuPanel(WINDOW* win, std::shared_ptr<I18nProvider> i18n = nullptr);
  ~MainMenuPanel();
  void render() override;
  void renderList();
  void renderStats();
  void renderSyncStateOnly();

  void setTodos(const std::vector<Todo>& todos);
  void setSelectedIndex(int index);
  void setScroll(int topOffset);
  void setCredentials(const std::string& username, const std::string& pantryId);
  void setUpdateVersion(const std::string& version);
  void setSyncStateData(const std::unordered_set<std::string>& newIds,
                        const std::unordered_set<std::string>& modifiedIds);

  int getMinWidth() const override { return Dimensions::MainMenuMinWidth; }
  int getMinHeight() const override { return Dimensions::MainMenuMinHeight; }
  int getVisibleRows() const;

 private:
  LoadingPanel loadingPanel;
  LogoPanel logoPanel;
  std::vector<Todo> todosList;

  int pointerIndex = 0;
  int moveFactor = 0;  // scroll offset
  std::string curUser;
  std::string pantryId;
  std::string updateVersion;  // Non-empty when update is ready

  std::unordered_set<std::string> syncNewIds;
  std::unordered_set<std::string> syncModifiedIds;

  std::string convertTimeToString(int epoch) const;

  WINDOW* todoUserName = nullptr;
  WINDOW* todoWindow = nullptr;
  WINDOW* todoBody = nullptr;
  void recreateWindows();
};
