#include "AddTodoPanel.h"
#include "utils/StringUtils.h"
#include <curses.h>
#include <algorithm>

// ─── Static member definition ─────────────────────────────────────────────────
std::string AddTodoPanel::clipboard_;

// ─── Layout constants ─────────────────────────────────────────────────────────
static constexpr int kNameSlotRows = 4;   // fixed visible rows for the name field
static constexpr int kDescLabelGap = 1;   // blank rows between name block and desc label

// ─── Constructor / destructor ─────────────────────────────────────────────────

AddTodoPanel::AddTodoPanel()
    : FullScreenPanel(), logoPanel(win, 0, 0), titleWin(nullptr), contentWin(nullptr) {}

AddTodoPanel::~AddTodoPanel() {
  if (titleWin)   delwin(titleWin);
  if (contentWin) delwin(contentWin);
}

void AddTodoPanel::setCredentials(const std::string& username, const std::string& id) {
  curUser  = username;
  pantryId = id;
}

// ─── Window management ────────────────────────────────────────────────────────

void AddTodoPanel::recreateWindows() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  int safe_w      = (max_x > 2) ? max_x - 2 : 1;
  int safe_h_menu = (max_y > 12) ? max_y - 12 : 1;

  if (!titleWin) {
    titleWin   = newwin(10, safe_w, 1, 1);
    contentWin = newwin(safe_h_menu, safe_w, 11, 1);
  } else {
    mvwin(titleWin,   0, 0);
    mvwin(contentWin, 0, 0);
#ifdef _WIN32
    resize_window(titleWin,   10, safe_w);
    resize_window(contentWin, safe_h_menu, safe_w);
#else
    wresize(titleWin,   10, safe_w);
    wresize(contentWin, safe_h_menu, safe_w);
#endif
    mvwin(titleWin,   1, 1);
    mvwin(contentWin, 11, 1);
  }
}

// ─── Static helpers ───────────────────────────────────────────────────────────

struct VisualRow {
  int charStart;
  int charEnd;
  std::string text;
};

static std::vector<VisualRow> buildVisualRows(const std::string& s, int fieldWidth) {
  std::vector<VisualRow> rows;
  if (fieldWidth <= 0) return rows;
  if (s.empty()) {
    rows.push_back({0, 0, ""});
    return rows;
  }
  int start = 0;
  int n = (int)s.size();
  while (start < n) {
    int len = 0;
    while (start + len < n && len < fieldWidth && s[start + len] != '\n') {
      len++;
    }
    rows.push_back({start, start + len, s.substr(start, len)});
    start += len;
    if (start < n && s[start] == '\n') {
      start++;
      if (start == n) {
        rows.push_back({start, start, ""});
      }
    }
  }
  return rows;
}

// Render a (possibly multi-line) string from scrollOff rows into the window.
static void renderWrappedScrolled(WINDOW* win, int startRow, int col,
                                   const std::vector<VisualRow>& rows, int fieldWidth,
                                   int scrollOff, int maxRow) {
  int row = startRow;
  for (int r = scrollOff; r < (int)rows.size() && row < maxRow; ++r, ++row) {
    std::string slice = rows[r].text;
    slice.resize(fieldWidth, ' ');   // pad to clear stale chars
    mvwprintw(win, row, col, "%s", slice.c_str());
  }
  // clear remaining rows in the slot
  while (row < maxRow) {
    std::string blank(fieldWidth, ' ');
    mvwprintw(win, row++, col, "%s", blank.c_str());
  }
}

// Apply A_REVERSE selection highlight on a visible field slot.
static void applySelHighlight(WINDOW* win, int startRow, int scrollOff,
                               int selMin, int selMax,
                               const std::vector<VisualRow>& rows, int fieldWidth, int col, int slotRows,
                               int contentBottom) {
  if (selMin >= selMax || rows.empty()) return;
  for (int r = 0; r < slotRows && (startRow + r) < contentBottom && (scrollOff + r) < (int)rows.size(); ++r) {
    const auto& vr = rows[scrollOff + r];
    int hlStart = std::max(selMin, vr.charStart) - vr.charStart;
    int hlEnd   = std::min(selMax, vr.charEnd)   - vr.charStart;
    if (hlStart < hlEnd) {
      mvwchgat(win, startRow + r, col + hlStart,
               hlEnd - hlStart, A_REVERSE, 0, nullptr);
    }
  }
}

// ─── render() ─────────────────────────────────────────────────────────────────

void AddTodoPanel::render() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  if (win && win != stdscr) {
#ifdef _WIN32
    resize_window(win, max_y, max_x);
#else
    wresize(win, max_y, max_x);
#endif
  }

  recreateWindows();

  wclear(titleWin);
  wclear(contentWin);
  box(titleWin,   0, 0);
  box(contentWin, 0, 0);

  // ── Header ──────────────────────────────────────────────────────────────────
  int part = (getmaxx(titleWin) - 81) / 4;
  if (part <= 0) part = 2;

  logoPanel.setWindow(titleWin);
  logoPanel.setPosition(1, part);
  logoPanel.render();

  int u_x = getmaxx(titleWin) - 50;
  if (u_x < part + 45) u_x = part + 45;
  int remainingW = getmaxx(titleWin) - u_x - 1;
  if (remainingW < 3) remainingW = 3;

  std::string dispUser = "Username: " + curUser;
  mvwprintw(titleWin, 3, u_x, "%s",
            StringUtils::truncateString(dispUser, remainingW).c_str());
  if (!pantryId.empty() && pantryId != "None") {
    std::string dispId = "Pantry ID: " + pantryId;
    mvwprintw(titleWin, 5, u_x, "%s",
              StringUtils::truncateString(dispId, remainingW).c_str());
  }

  // ── Input layout ────────────────────────────────────────────────────────────
  int cw           = getmaxx(contentWin);
  int ch           = getmaxy(contentWin);
  int fieldWidth   = (cw > 4) ? cw - 4 : 1;
  int col          = 2;
  int contentBottom = ch - 1;

  // Compute effective scroll offsets:
  //   - During editing (activeField_ > 0) use cursor-driven stored offsets.
  //   - Otherwise scroll to show end of text so the last characters are visible.
  int effNameScroll, effDescScroll;
  int descLabelRow = 3 + kNameSlotRows + kDescLabelGap;
  int descStartRow = descLabelRow + 1;
  int descAvailRows = contentBottom - descStartRow;
  if (descAvailRows < 1) descAvailRows = 1;

  std::vector<VisualRow> nameRows = buildVisualRows(name, fieldWidth);
  std::vector<VisualRow> descRows = buildVisualRows(desc, fieldWidth);

  if (activeField_ == 0) {
    effNameScroll = std::max(0, (int)nameRows.size() - kNameSlotRows);
    effDescScroll = std::max(0, (int)descRows.size() - descAvailRows);
  } else {
    effNameScroll = nameScroll_;
    effDescScroll = descScroll_;
  }

  if (ch > 3) {
    // ── Name ────────────────────────────────────────────────────────────────
    mvwprintw(contentWin, 2, col, "Enter Todo Name (empty to skip):");
    renderWrappedScrolled(contentWin, 3, col, nameRows, fieldWidth,
                          effNameScroll, 3 + kNameSlotRows);

    // Selection highlight for name
    if (activeField_ == 1 && anchor_ >= 0 && anchor_ != cursor_) {
      int sMin = std::min(cursor_, anchor_);
      int sMax = std::max(cursor_, anchor_);
      applySelHighlight(contentWin, 3, effNameScroll, sMin, sMax,
                        nameRows, fieldWidth, col, kNameSlotRows, contentBottom);
    }

    // ── Desc ────────────────────────────────────────────────────────────────
    if (descLabelRow < contentBottom) {
      mvwprintw(contentWin, descLabelRow, col, "Enter Todo Description:");
      renderWrappedScrolled(contentWin, descStartRow, col, descRows, fieldWidth,
                            effDescScroll, contentBottom);

      // Selection highlight for desc
      if (activeField_ == 2 && anchor_ >= 0 && anchor_ != cursor_) {
        int sMin = std::min(cursor_, anchor_);
        int sMax = std::max(cursor_, anchor_);
        applySelHighlight(contentWin, descStartRow, effDescScroll, sMin, sMax,
                          descRows, fieldWidth, col, descAvailRows, contentBottom);
      }
    }
  }

  if (statsPanel) {
    statsPanel->setWindow(win);
    statsPanel->render();
  }

  wrefresh(titleWin);
  wrefresh(contentWin);

  // Key bar varies depending on whether we're in an input field
  if (activeField_ > 0) {
    FullScreenPanel::refreshKeyBar({
      {"Enter",   "Next field"},
      {"Esc",     "Cancel"}
    });
  } else {
    FullScreenPanel::refreshKeyBar({
      {"Enter", "Next field"},
      {"Esc/^C", "Cancel/Exit"},
    });
  }
}

// ─── captureInput() ───────────────────────────────────────────────────────────

std::string AddTodoPanel::captureInput(bool isNameField) {
  std::string& str = isNameField ? name : desc;

  // ── Initialise editing state ───────────────────────────────────────────────
  cursor_      = (int)str.size();   // start caret at end
  anchor_      = -1;
  activeField_ = isNameField ? 1 : 2;
  if (isNameField) nameScroll_ = 0;
  else             descScroll_ = 0;

  keypad(contentWin, TRUE);
  curs_set(1);

  // Switch to raw mode so Ctrl+C is delivered as char 3 (copy) rather than SIGINT.
  // noraw() / cbreak() is called before we return to restore normal signal handling.
  raw();

  // ── Helpers ───────────────────────────────────────────────────────────────

  auto getFieldWidth = [&]() {
    int cw = getmaxx(contentWin);
    return (cw > 4) ? cw - 4 : 1;
  };

  auto getDescAvailRows = [&]() {
    int descLabelRow = 3 + kNameSlotRows + kDescLabelGap;
    int descStartRow = descLabelRow + 1;
    int h = getmaxy(contentWin) - 1 - descStartRow;
    return (h > 1) ? h : 1;
  };

  auto hasSelection = [&]() { return anchor_ >= 0 && anchor_ != cursor_; };
  auto selMin       = [&]() { return std::min(cursor_, anchor_); };
  auto selMax       = [&]() { return std::max(cursor_, anchor_); };

  auto deleteSelection = [&]() {
    str.erase(selMin(), selMax() - selMin());
    cursor_ = selMin();
    anchor_ = -1;
  };

  auto getCursorPos = [&](int& cRow, int& cCol) {
    std::vector<VisualRow> rows = buildVisualRows(str, getFieldWidth());
    cRow = 0;
    cCol = 0;
    for (int i = 0; i < (int)rows.size(); ++i) {
      if (cursor_ >= rows[i].charStart && cursor_ <= rows[i].charEnd) {
        cRow = i;
        cCol = cursor_ - rows[i].charStart;
        if (cursor_ == rows[i].charEnd && i + 1 < (int)rows.size() && rows[i+1].charStart == cursor_) {
          continue; // Wrapped exactly, prefer next line
        }
        break;
      }
    }
  };

  // Adjust scroll so cursor_ stays visible in the viewport
  auto updateScroll = [&]() {
    int curRow = 0, curCol = 0;
    getCursorPos(curRow, curCol);
    if (isNameField) {
      if (curRow < nameScroll_)
        nameScroll_ = curRow;
      if (curRow >= nameScroll_ + kNameSlotRows)
        nameScroll_ = curRow - kNameSlotRows + 1;
      if (nameScroll_ < 0) nameScroll_ = 0;
    } else {
      int avail = getDescAvailRows();
      if (curRow < descScroll_)
        descScroll_ = curRow;
      if (curRow >= descScroll_ + avail)
        descScroll_ = curRow - avail + 1;
      if (descScroll_ < 0) descScroll_ = 0;
    }
  };

  // Screen coordinates of the caret
  auto screenCursor = [&](int& cy, int& cx) {
    int curRow = 0, curCol = 0;
    getCursorPos(curRow, curCol);
    int col = 2;
    if (isNameField) {
      cy = 3 + (curRow - nameScroll_);
      cx = col + curCol;
    } else {
      int descLabelRow = 3 + kNameSlotRows + kDescLabelGap;
      int descStartRow = descLabelRow + 1;
      cy = descStartRow + (curRow - descScroll_);
      cx = col + curCol;
    }
    int maxY = getmaxy(contentWin) - 1;
    int maxX = getmaxx(contentWin) - 1;
    if (cy > maxY) cy = maxY;
    if (cy < 0)   cy = 0;
    if (cx > maxX) cx = maxX;
    if (cx < 0)   cx = 0;
  };

  // ── Word-boundary helpers ────────────────────────────────────────────────────
  auto prevWordStart = [&]() -> int {
    int p = cursor_;
    while (p > 0 && str[p - 1] == ' ') --p;   // skip trailing spaces
    while (p > 0 && str[p - 1] != ' ') --p;   // skip word chars
    return p;
  };

  auto nextWordEnd = [&]() -> int {
    int p = cursor_;
    while (p < (int)str.size() && str[p] != ' ') ++p;  // skip word chars
    while (p < (int)str.size() && str[p] == ' ')  ++p;  // skip spaces
    return p;
  };

  // ── Initial draw ──────────────────────────────────────────────────────────
  updateScroll();
  render();
  int cy, cx;
  screenCursor(cy, cx);
  wmove(contentWin, cy, cx);
  wrefresh(contentWin);

  // ── Input loop ─────────────────────────────────────────────────────────────
  while (true) {
    int ch = wgetch(contentWin);
    if (ch == ERR) continue;

    // Read modifier keys — PDCurses only; on ncurses mods are always 0.
#ifdef __PDCURSES__
    unsigned long mods = PDC_get_key_modifiers();
    bool rawShiftHeld = (mods & PDC_KEY_MODIFIER_SHIFT)   != 0;
    bool ctrlHeld     = (mods & PDC_KEY_MODIFIER_CONTROL) != 0;
#else
    bool rawShiftHeld = false;
    bool ctrlHeld     = false;
#endif
    (void)ctrlHeld;  // suppress unused-variable warning on non-PDCurses builds

    // Some terminals encode Shift directly into the keycode (KEY_SLEFT, KEY_SRIGHT, etc.)
    bool isShiftKey = (ch == KEY_SLEFT || ch == KEY_SRIGHT || ch == KEY_SHOME || ch == KEY_SEND);
    bool shiftHeld  = rawShiftHeld || isShiftKey;

    // Ctrl+Left / Ctrl+Shift+Left: word jump or word select
    bool isCtrlLeft  = (ch == CTL_LEFT)  || ((ch == KEY_LEFT || ch == KEY_SLEFT) && ctrlHeld);
    
    // Ctrl+Right / Ctrl+Shift+Right
    bool isCtrlRight = (ch == CTL_RIGHT) || ((ch == KEY_RIGHT || ch == KEY_SRIGHT) && ctrlHeld);

    // Ctrl+Backspace: delete previous word
    bool isCtrlBksp  = (ch == CTL_BKSP) ||
                       ((ch == KEY_BACKSPACE || ch == 127 || ch == '\b') && ctrlHeld);

    // Terminal resize
    if (ch == KEY_RESIZE) {
#ifdef _WIN32
      FullScreenPanel::handleResize();
#else
      wclear(win);
      FullScreenPanel::show();
#endif
      updateScroll();
      render();
      screenCursor(cy, cx);
      wmove(contentWin, cy, cx);
      wrefresh(contentWin);
      continue;
    }

    // ── Ctrl+Backspace: delete previous word ──────────────────────────────────
    if (isCtrlBksp) {
      if (hasSelection()) {
        deleteSelection();
      } else if (cursor_ > 0) {
        int ws = prevWordStart();
        str.erase(ws, cursor_ - ws);
        cursor_ = ws;
      }

    // ── Ctrl+Left / Ctrl+Shift+Left: word jump / word select ─────────────────
    } else if (isCtrlLeft) {
      int ws = prevWordStart();
      if (shiftHeld) {
        if (anchor_ < 0) anchor_ = cursor_;
        cursor_ = ws;
        if (cursor_ == anchor_) anchor_ = -1;
      } else {
        anchor_ = -1;
        cursor_ = ws;
      }

    // ── Ctrl+Right / Ctrl+Shift+Right: word jump / word select ───────────────
    } else if (isCtrlRight) {
      int we = nextWordEnd();
      if (shiftHeld) {
        if (anchor_ < 0) anchor_ = cursor_;
        cursor_ = we;
        if (cursor_ == anchor_) anchor_ = -1;
      } else {
        anchor_ = -1;
        cursor_ = we;
      }

    // ── Regular LEFT / RIGHT ──────────────────────────────────────────────────
    } else if (ch == KEY_LEFT) {
      if (hasSelection()) { cursor_ = selMin(); anchor_ = -1; }
      else if (cursor_ > 0) --cursor_;

    } else if (ch == KEY_RIGHT) {
      if (hasSelection()) { cursor_ = selMax(); anchor_ = -1; }
      else if (cursor_ < (int)str.size()) ++cursor_;

    } else if (ch == KEY_HOME) {
      anchor_ = -1;
      cursor_ = 0;

    } else if (ch == KEY_END) {
      anchor_ = -1;
      cursor_ = (int)str.size();

    // ── Selection extension (Shift + arrow) ──────────────────────────────────
    } else if (ch == KEY_SLEFT) {
      if (anchor_ < 0) anchor_ = cursor_;
      if (cursor_ > 0) --cursor_;
      if (cursor_ == anchor_) anchor_ = -1;

    } else if (ch == KEY_SRIGHT) {
      if (anchor_ < 0) anchor_ = cursor_;
      if (cursor_ < (int)str.size()) ++cursor_;
      if (cursor_ == anchor_) anchor_ = -1;

    } else if (ch == KEY_SHOME) {
      if (anchor_ < 0) anchor_ = cursor_;
      cursor_ = 0;
      if (cursor_ == anchor_) anchor_ = -1;

    } else if (ch == KEY_SEND) {
      if (anchor_ < 0) anchor_ = cursor_;
      cursor_ = (int)str.size();
      if (cursor_ == anchor_) anchor_ = -1;

    // ── Ctrl+A: select all ────────────────────────────────────────────────────
    } else if (ch == 1 || ((ch == 'a' || ch == 'A') && ctrlHeld)) {
      anchor_ = 0;
      cursor_ = (int)str.size();

    // ── Ctrl+C: copy ─────────────────────────────────────────────────────────
    } else if (ch == 3 || ((ch == 'c' || ch == 'C') && ctrlHeld)) {
      if (hasSelection()) {
        std::string text = str.substr(selMin(), selMax() - selMin());
#ifdef __PDCURSES__
        PDC_setclipboard(text.c_str(), (long)text.size());
#else
        clipboard_ = text;
#endif
      }

    // ── Ctrl+X: cut ──────────────────────────────────────────────────────────
    } else if (ch == 24 || ((ch == 'x' || ch == 'X') && ctrlHeld)) {
      if (hasSelection()) {
        std::string text = str.substr(selMin(), selMax() - selMin());
#ifdef __PDCURSES__
        PDC_setclipboard(text.c_str(), (long)text.size());
#else
        clipboard_ = text;
#endif
        deleteSelection();
      }

    // ── Ctrl+V: paste ─────────────────────────────────────────────────────────
    } else if (ch == 22 || ((ch == 'v' || ch == 'V') && ctrlHeld)) {
      if (hasSelection()) deleteSelection();
#ifdef __PDCURSES__
      char* clipText = nullptr;
      long clipLen = 0;
      if (PDC_getclipboard(&clipText, &clipLen) == PDC_CLIP_SUCCESS) {
        if (clipText && clipLen > 0) {
          str.insert(cursor_, clipText, clipLen);
          cursor_ += clipLen;
        }
        if (clipText) PDC_freeclipboard(clipText);
      }
#else
      if (!clipboard_.empty()) {
        str.insert(cursor_, clipboard_);
        cursor_ += (int)clipboard_.size();
      }
#endif

    // ── Ctrl+W: REMOVED — use Ctrl+Backspace instead ─────────────────────────
    // (ch == 23 intentionally not handled here)

    // ── Up / Down arrow: vertical cursor movement ─────────────────────────────
    } else if (ch == KEY_UP || ch == KEY_DOWN) {
      if (!shiftHeld) anchor_ = -1;
      int curRow = 0, curCol = 0;
      getCursorPos(curRow, curCol);
      std::vector<VisualRow> rows = buildVisualRows(str, getFieldWidth());
      int oldCursor = cursor_;
      
      if (ch == KEY_UP) {
        if (curRow > 0) {
          cursor_ = std::min(rows[curRow - 1].charStart + curCol, rows[curRow - 1].charEnd);
        } else {
          cursor_ = 0;
        }
      } else if (ch == KEY_DOWN) {
        if (curRow < (int)rows.size() - 1) {
          cursor_ = std::min(rows[curRow + 1].charStart + curCol, rows[curRow + 1].charEnd);
        } else {
          cursor_ = (int)str.size();
        }
      }

      if (shiftHeld && anchor_ < 0) anchor_ = oldCursor;
      if (cursor_ == anchor_) anchor_ = -1;

    // ── Delete key: delete next char ─────────────────────────────────────────
    } else if (ch == KEY_DC) {
      if (hasSelection()) {
        deleteSelection();
      } else if (cursor_ < (int)str.size()) {
        str.erase(cursor_, 1);
      }

    // ── Backspace: delete previous char ──────────────────────────────────────
    } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
      if (hasSelection()) {
        deleteSelection();
      } else if (cursor_ > 0) {
        str.erase(cursor_ - 1, 1);
        --cursor_;
      }

    // ── Enter / carriage-return: commit field (raw mode delivers \r not \n) ──
    } else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
      if (!isNameField && shiftHeld) {
        if (hasSelection()) deleteSelection();
        str.insert(cursor_, 1, '\n');
        ++cursor_;
      } else {
        break;
      }

    // ── Escape: cancel input entirely ─────────────────────────────────────────
    } else if (ch == 27) {
      str = "\x1b";
      break;

    // ── Printable character ───────────────────────────────────────────────────
    } else if (isprint(ch)) {
      if (hasSelection()) deleteSelection();
      str.insert(cursor_, 1, static_cast<char>(ch));
      ++cursor_;
    }

    // Clamp cursor just in case
    if (cursor_ < 0)             cursor_ = 0;
    if (cursor_ > (int)str.size()) cursor_ = (int)str.size();

    updateScroll();
    render();
    screenCursor(cy, cx);
    wmove(contentWin, cy, cx);
    wrefresh(contentWin);
  }

  // Restore character-at-a-time mode with signal processing re-enabled
  noraw();
  cbreak();

  activeField_ = 0;
  anchor_      = -1;
  return str;
}

// ─── promptInput() ────────────────────────────────────────────────────────────

void AddTodoPanel::promptInput() {
  noecho();
  curs_set(1);

  name = "";
  desc = "";

  FullScreenPanel::show();

  std::string n = captureInput(true);
  if (n == "\x1b") {
    name = "";
    desc = "";
  } else {
    std::string d = captureInput(false);
    if (d == "\x1b") {
      name = "";
      desc = "";
    }
  }

  curs_set(0);

  wclear(win);
  wrefresh(win);
}

std::string AddTodoPanel::getEnteredName() const { return name; }
std::string AddTodoPanel::getEnteredDesc() const { return desc; }
