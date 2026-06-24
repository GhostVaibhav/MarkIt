#pragma once
#include <string>
#include <curses.h>

#include "DimensionConfig.h"
#include "FullScreenPanel.h"
#include "LogoPanel.h"

class AddTodoPanel : public FullScreenPanel {
 public:
  explicit AddTodoPanel();
  ~AddTodoPanel() override;

  void render() override;
  void promptInput();

  int getMinWidth() const override { return Dimensions::AddTodoMinWidth; }
  int getMinHeight() const override { return Dimensions::AddTodoMinHeight; }

  std::string getEnteredName() const;
  std::string getEnteredDesc() const;

  void setCredentials(const std::string& username, const std::string& id);

 private:
  std::string name;
  std::string desc;

  std::string curUser;
  std::string pantryId;

  LogoPanel logoPanel;
  WINDOW* titleWin  = nullptr;
  WINDOW* contentWin = nullptr;

  // ── Live editing state (valid during captureInput) ─────────────────────────
  int cursor_      = 0;   // caret position in active field string (char index)
  int anchor_      = -1;  // selection anchor; -1 = no selection
  int activeField_ = 0;   // 0 = idle, 1 = name, 2 = desc
  int nameScroll_  = 0;   // rows hidden above name viewport
  int descScroll_  = 0;   // rows hidden above desc viewport
  static std::string clipboard_;  // in-process clipboard shared across instances

  void recreateWindows();
  std::string captureInput(bool isNameField);
};
