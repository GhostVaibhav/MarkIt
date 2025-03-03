/*
 *      __  ___         __    ____ __  __
 *     /  |/  /__ _____/ /__ /  _// /_/ /
 *    / /|_/ / _ `/ __/  '_/_/ / / __/_/
 *   /_/  /_/\_,_/_/ /_/\_\/___/ \__(_)
 *
 *   MIT License
 *
 *   Copyright (c) 2025 Vaibhav Sharma
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "panels.h"
#include "backend.h"
#include "cloudFunctions.h"
#include "curses.h"
#include "fileHandlers.h"
#include "globalVariable.h"
#include "logger.h"
#include "sha256.h"
#include "structure.h"

void PrintCenter(const unsigned int *selected, std::vector<std::string> a, WINDOW *win) {
  const int maxSize = max_element(a.begin(), a.end())->size();
  for (int i = 0; i < a.size(); i++) {
    if (i != *selected)
      mvwprintw(win, (getmaxy(win) / 2) + (i - (a.size() / 2)), (getmaxx(win) / 2) - maxSize, a[i].c_str());
    else {
      wattron(win, COLOR_PAIR(1));
      mvwprintw(win, (getmaxy(win) / 2) + (i - (a.size() / 2)), (getmaxx(win) / 2) - maxSize, a[i].c_str());
      wattroff(win, COLOR_PAIR(1));
    }
  }
}

unsigned int MenuPanel(const std::vector<std::string> a) {
  WINDOW *title = newwin(10, getmaxx(stdscr) - 2, 1, 1);
  WINDOW *menu = newwin(getmaxy(stdscr) - 12, getmaxx(stdscr) - 2, 11, 1);
  unsigned int pointerIndex = 0;
  int c;
  keypad(stdscr, true);
  refresh();
  while (true) {
    curs_set(0);
    resizeEvent();
#ifdef _WIN32
    resize_window(title, 10, getmaxx(stdscr) - 2);
    resize_window(menu, getmaxy(stdscr) - 12, getmaxx(stdscr) - 2);
#else
    wresize(title, 10, getmaxx(stdscr) - 2);
    wresize(menu, getmaxy(stdscr) - 12, getmaxx(stdscr) - 2);
#endif
    if (getmaxx(stdscr) >= minWidth && getmaxy(stdscr) >= minHeight) {
      int part = (getmaxx(title) - 81) / 4;
      constexpr int half = 1;
      if (part <= 0)
        part = 1;
      wclear(title);
      wclear(menu);
      wrefresh(title);
      wrefresh(menu);
      box(title, 0, 0);
      box(menu, 0, 0);
      LogoPanel(title, half, part);
      mvwprintw(title, 3, 3 * part + 25, ("Username: " + curUser).c_str());
      mvwprintw(title, 5, 3 * part + 25, ("Pantry ID: " + pantryId).c_str());
      PrintCenter(&pointerIndex, a, menu);
      refresh();
      StatsPanel(stdscr);
      wrefresh(title);
      wrefresh(menu);
    } else {
      if (getmaxx(stdscr) < minWidth)
        mvprintw(LINES / 2, (COLS - 35) / 2, "Please increase your window's width");
      else
        mvprintw(LINES / 2, (COLS - 36) / 2, "Please increase your window's height");
    }
    c = getch();
    switch (c) {
      case KEY_UP:
        pointerIndex--;
        if (pointerIndex == -1)
          pointerIndex = 0;
        break;
      case KEY_DOWN:
        pointerIndex++;
        if (pointerIndex == a.size())
          pointerIndex = a.size() - 1;
        break;
      case '\b':
        return -1;
      case '\n':
        endwin();
        return pointerIndex;
      default:
        refresh();
        break;
    }
    refresh();
  }
}

void MainMenuPanel() {
  curs_set(0);
  WINDOW *todoUserName = newwin(10, getmaxx(stdscr) - 2, 1, 1);
  WINDOW *todoWindow = newwin(getmaxy(stdscr) - 12, getmaxx(stdscr) - 2, 11, 1);
  WINDOW *todoBody = newwin(getmaxy(todoWindow) - 4, getmaxx(todoWindow) - 1, getmaxy(todoUserName) + 4, 1);
  int c = KEY_RESIZE, moveFactor = 0, bottomT = 0, topT = getmaxy(todoBody);
  unsigned int pointerIndex = 0;
  keypad(todoWindow, true);
  while (true) {
    std::vector<todo> temp = localSave["data"];
    noecho();
    curs_set(0);
    if (c == KEY_RESIZE) {
      resizeEvent();
#ifdef _WIN32
      resize_window(todoWindow, getmaxy(stdscr) - 12, getmaxx(stdscr) - 2);
      resize_window(todoUserName, 10, getmaxx(stdscr) - 2);
      resize_window(todoBody, getmaxy(todoWindow) - 4, getmaxx(todoWindow) - 1);
#else
      wresize(todoWindow, getmaxy(stdscr) - 12, getmaxx(stdscr) - 2);
      wresize(todoUserName, 10, getmaxx(stdscr) - 2);
      wresize(todoBody, getmaxy(todoWindow) - 4, getmaxx(todoWindow) - 1);
#endif
    }
    if (getmaxx(stdscr) >= minWidth && getmaxy(stdscr) >= minHeight) {
      int part = (getmaxx(todoUserName) - 81) / 4;
      int half = 1;
      if (part <= 0)
        part = 1;
      if (c == KEY_RESIZE) {
        wclear(todoWindow);
        wclear(todoUserName);
        wrefresh(todoWindow);
        box(todoWindow, 0, 0);
        box(todoUserName, 0, 0);
      }
      wclear(todoBody);
      wrefresh(todoBody);
      refresh();
      LogoPanel(todoUserName, half, part);
      mvwprintw(todoUserName, 3, 3 * part + 25, ("Username: " + curUser).c_str());
      mvwprintw(todoUserName, 5, 3 * part + 25, ("Pantry ID: " + pantryId).c_str());
      int tabDiv = (getmaxx(todoWindow) - 2) / 3;
      mvwvline(todoWindow, 1, tabDiv, 0, 1);
      mvwvline(todoWindow, 1, 2 * tabDiv, 0, 1);
      mvwhline(todoWindow, 2, 1, 0, getmaxx(todoWindow) - 2);
      mvwprintw(todoWindow, 1, (tabDiv - 4) / 2, "Name");
      mvwprintw(todoWindow, 1, ((3 * tabDiv - 12) / 2) + 1, "Description");
      mvwprintw(todoWindow, 1, ((5 * tabDiv - 13) / 2) + 2, "Created Time");
      BORDER(todoWindow);
      for (int i = 0; i < temp.size(); i++) {
        if (pointerIndex == i) {
          if (temp.at(i).isComplete)
            wattron(todoBody, COLOR_PAIR(2));
          else
            wattron(todoBody, COLOR_PAIR(1));
        }
        if (!has_colors())
          wattron(todoBody, A_REVERSE);
        mvwprintw(todoBody, i + moveFactor, (tabDiv - temp[i].name.size()) / 2, (temp[i].name).c_str());
        mvwprintw(todoBody, i + moveFactor, ((3 * tabDiv - temp[i].desc.size()) / 2) + 1, (temp[i].desc).c_str());
        mvwprintw(todoBody, i + moveFactor, ((5 * tabDiv - temp[i].time.size()) / 2) + 2, (temp[i].time).c_str());
        if (pointerIndex == i) {
          if (temp[i].isComplete)
            wattroff(todoBody, COLOR_PAIR(2));
          else
            wattroff(todoBody, COLOR_PAIR(1));
        }
        if (!has_colors())
          wattroff(todoBody, A_REVERSE);
        BORDER(todoWindow);
      }
      refresh();
      StatsPanel(stdscr);
      wrefresh(todoWindow);
      wrefresh(todoUserName);
      wrefresh(todoBody);
    } else {
      if (getmaxx(stdscr) < minWidth)
        mvprintw(LINES / 2, (COLS - 35) / 2, "Please increase your window's width");
      else
        mvprintw(LINES / 2, (COLS - 36) / 2, "Please increase your window's height");
    }
    c = getch();
    switch (c) {
      case KEY_UP:
        pointerIndex--;
        if (pointerIndex < bottomT) {
          bottomT--;
          topT--;
          moveFactor++;
        }
        break;
      case KEY_DOWN:
        pointerIndex++;
        if (pointerIndex >= temp.size())
          pointerIndex = temp.size() - 1;
        if (pointerIndex >= topT) {
          bottomT++;
          topT++;
          moveFactor--;
        }
        break;
      case 'M':
      case 'm': {
        refresh();
        echo();
        const unsigned int choice =
                MenuPanel({"1. Add a todo", "2. Push all changes", "3. Pull from the cloud", "4. Refresh"});
        if (choice == -1) {
          break;
        }
        if (choice == 0) {
          c = AddTodo(todoWindow);
          while (!c)
            c = AddTodo(todoWindow);
        } else if (choice == 1) {
          PushToCloud(stdscr);
        } else if (choice == 2) {
          PullFromCloud();
        } else if (choice == 3)
          RefreshCloudSave();
      }
        noecho();
        c = KEY_RESIZE;
        break;
      case '\b':
        return;
      case '\n': {
        refresh();
        echo();
        const unsigned int choice = MenuPanel({"1. Delete", "2. Mark/Unmark"});
        if (choice == -1) {
          break;
        }
        if (choice == 0) {
          if (DeleteTodo(&pointerIndex)) {
            LoadingPanel("Todo deleted");
          } else {
            LoadingPanel("Todo could not be deleted");
          }
        } else if (choice == 1) {
          temp.at(pointerIndex).isComplete = !temp.at(pointerIndex).isComplete;
          localSave["data"] = temp;
          _write_to_file(localSave, storageFile);
        }
      }
        noecho();
        c = KEY_RESIZE;
        break;
      default:
        break;
    }
  }
}

int LoginPanel(std::string *bucket) {
  curs_set(0);
  int part = (getmaxy(stdscr) - 18) / 4;
  char userName[32];
  WINDOW *title = newwin(8, getmaxx(stdscr), part, 0);
  WINDOW *userNameWindow = newwin(5, getmaxx(stdscr) - 20, 2 * part + 8, 10);
  WINDOW *passwordWindow = newwin(5, getmaxx(stdscr) - 20, 3 * part + 13, 10);
  WINDOW *information = newwin(3, getmaxx(stdscr), getmaxy(stdscr) - 3, 0);
  WINDOW *wrongPassword = newwin(3, getmaxx(stdscr), 10, 0);
  while (true) {
    char password[64];
    curs_set(0);
    clear();
    resizeEvent();
#ifdef _WIN32
    resize_window(userNameWindow, 5, getmaxx(stdscr) - 20);
    resize_window(passwordWindow, 5, getmaxx(stdscr) - 20);
    resize_window(title, 8, getmaxx(stdscr));
    resize_window(information, 3, getmaxx(stdscr));
    resize_window(wrongPassword, 3, getmaxx(stdscr));
#else
    wresize(userNameWindow, 5, getmaxx(stdscr) - 20);
    wresize(passwordWindow, 5, getmaxx(stdscr) - 20);
    wresize(title, 8, getmaxx(stdscr));
    wresize(information, 3, getmaxx(stdscr));
    wresize(wrongPassword, 3, getmaxx(stdscr));
#endif
    if (getmaxx(stdscr) >= minWidth && getmaxy(stdscr) >= minHeight) {
      wclear(userNameWindow);
      wclear(passwordWindow);
      wclear(title);
      wclear(information);
      wrefresh(userNameWindow);
      wrefresh(passwordWindow);
      wborder(stdscr, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
      BORDER(userNameWindow);
      BORDER(passwordWindow);
      part = (getmaxx(title) - 34) / 2;
      int half = 0;
      LogoPanel(title, half, part);
      mvwprintw(information, 1, (getmaxx(information) - 26) / 2, "Don't resize this window!");
      mvwprintw(userNameWindow, getmaxy(userNameWindow) / 2, 5, "Username: ");
      mvwprintw(passwordWindow, getmaxy(passwordWindow) / 2, (getmaxx(passwordWindow) - 20) / 2, "Enter your password");
      refresh();
      wrefresh(title);
      wrefresh(information);
      wrefresh(wrongPassword);
      wbkgd(userNameWindow, COLOR_PAIR(1));
      wgetstr(userNameWindow, userName);
      wbkgd(userNameWindow, COLOR_PAIR(6));
      BORDER(userNameWindow);
      wrefresh(userNameWindow);
      noecho();
      wbkgd(passwordWindow, COLOR_PAIR(1));
      wgetstr(passwordWindow, password);
      wbkgd(passwordWindow, COLOR_PAIR(6));
      echo();
      wrefresh(userNameWindow);
      wrefresh(passwordWindow);
    } else {
      if (getmaxx(stdscr) < minWidth)
        mvprintw(LINES / 2, (COLS - 35) / 2, "Please increase your window's width");
      else
        mvprintw(LINES / 2, (COLS - 36) / 2, "Please increase your window's height");
    }
    std::string userNameString = userName;
    std::string passwordString = password;
    passwordString = sha256(passwordString);
    LoadingPanel("Generating keys...");
    GetAPIKey(userNameString);
    logger->info("Got the API keys, and written to file");
    nlohmann::json temp;
    temp["key"] = pantryId;
    _write_to_file(temp, keyFile);
    LoadingPanel("Loading bucket details...");
    try {
      if (getBucket(userNameString)) {
        cloudSave = getBucketDetails(userNameString);
        if (cloudSave["hash"] == passwordString) {
          clear();
          *bucket = userName;
          curUserHash = passwordString;
          json jsons;
          jsons["userName"] = userName;
          jsons["userHash"] = passwordString;
          _write_to_file(jsons, stateFile);
          return 2;
        }
        wattron(wrongPassword, COLOR_PAIR(2));
        mvwprintw(wrongPassword, 2, (getmaxx(wrongPassword) - 15) / 2, "Wrong Password");
        wattroff(wrongPassword, COLOR_PAIR(2));
      } else {
        std::vector<todo> t;
        cloudSave["hash"] = passwordString;
        cloudSave["number"] = 0;
        cloudSave["data"] = t;
        createBucket(userNameString);
        replaceBucket(userNameString, cloudSave);
        clear();
        *bucket = userName;
        json temp;
        temp["userName"] = userName;
        temp["userHash"] = passwordString;
        _write_to_file(temp, stateFile);
        return 1;
      }
    } catch (const char *err) {
      logger->error("Error: " + std::string(err) + " while logging in");
      std::string a = err;
      wattron(wrongPassword, COLOR_PAIR(2));
      mvwprintw(wrongPassword, 2, (getmaxx(wrongPassword) - a.size()) / 2, a.c_str());
      wattroff(wrongPassword, COLOR_PAIR(2));
    }
  }
}

void WelcomePanel(const int code) {
  if (code == -1)
    return;
  WINDOW *loading = newwin(0, 0, 0, 0);
  clear();
  int part = (getmaxx(stdscr) - 36) / 2;
  int half = ((getmaxy(stdscr) - 4) / 2) - 1;
  refresh();
  wrefresh(loading);
  LogoPanel(loading, half, part);
  if (code == 1)
    mvwprintw(loading, getmaxy(stdscr) - 2, (getmaxx(stdscr) - 10 - curUser.size()) / 2,
              ("Welcome, " + curUser).c_str());
  else if (code == 2)
    mvwprintw(loading, getmaxy(stdscr) - 2, (getmaxx(stdscr) - 15 - curUser.size()) / 2,
              ("Welcome back, " + curUser).c_str());
  wrefresh(loading);
  refresh();
  wgetch(loading);
}

void LogoPanel(WINDOW *win, int x, int y) noexcept {
  wattron(win, COLOR_PAIR(3));
  mvwprintw(win, x + 1, y, R"(   __  ___)");
  mvwprintw(win, x + 2, y, R"(  /  |/  /)");
  mvwprintw(win, x + 3, y, R"( / /|_/ /)");
  mvwprintw(win, x + 4, y, R"(/_/  /_/)");
  wattroff(win, COLOR_PAIR(3));
  wattron(win, COLOR_PAIR(4));
  mvwprintw(win, x + 1, y + 7, R"()");
  mvwprintw(win, x + 2, y + 10, R"(__ _)");
  mvwprintw(win, x + 3, y + 9, R"( _ `)");
  mvwprintw(win, x + 4, y + 8, R"(\_,_)");
  wattroff(win, COLOR_PAIR(4));
  wattron(win, COLOR_PAIR(4));
  mvwprintw(win, x + 1, y + 14, R"()");
  mvwprintw(win, x + 2, y + 14, R"(____)");
  mvwprintw(win, x + 3, y + 13, R"(/ __)");
  mvwprintw(win, x + 4, y + 12, R"(/_/ )");
  wattroff(win, COLOR_PAIR(4));
  wattron(win, COLOR_PAIR(2));
  mvwprintw(win, x + 1, y + 19, R"(__    )");
  mvwprintw(win, x + 2, y + 18, R"(/ /__ )");
  mvwprintw(win, x + 3, y + 17, R"(/  '_/)");
  mvwprintw(win, x + 4, y + 16, R"(/_/\_\)");
  wattroff(win, COLOR_PAIR(2));
  wattron(win, COLOR_PAIR(5));
  mvwprintw(win, x + 1, y + 25, R"(____)");
  mvwprintw(win, x + 2, y + 24, R"(/  _)");
  mvwprintw(win, x + 3, y + 23, R"(_/ /)");
  mvwprintw(win, x + 4, y + 22, R"(/___/)");
  wattroff(win, COLOR_PAIR(5));
  wattron(win, COLOR_PAIR(5));
  mvwprintw(win, x + 1, y + 29, R"(__  )");
  mvwprintw(win, x + 2, y + 28, R"(/ /_)");
  mvwprintw(win, x + 3, y + 27, R"(/ __)");
  mvwprintw(win, x + 4, y + 27, R"(\__)");
  wattroff(win, COLOR_PAIR(5));
  wattron(win, COLOR_PAIR(1));
  mvwprintw(win, x + 1, y + 33, R"(__)");
  mvwprintw(win, x + 2, y + 32, R"(/ /)");
  mvwprintw(win, x + 3, y + 31, R"(/_/)");
  mvwprintw(win, x + 4, y + 30, R"((v))");
  mvwprintw(win, x + 4, y + 33, (APP_VERSION).c_str());
  wattroff(win, COLOR_PAIR(1));
}

void LoadingPanel(const std::string &loadText) {
  clear();
  int part = (getmaxx(stdscr) - 36) / 2;
  int half = ((getmaxy(stdscr) - 4) / 2) - 1;
  refresh();
  wrefresh(stdscr);
  LogoPanel(stdscr, half, part);
  mvwprintw(stdscr, getmaxy(stdscr) - 2, (getmaxx(stdscr) - loadText.size()) / 2, loadText.c_str());
  wrefresh(stdscr);
  refresh();
}

void resizeEvent() {
  int w, h;
  resize_term(0, 0);
  getmaxyx(stdscr, h, w);
  clear();
  BORDER(stdscr);
}

void StatsPanel(WINDOW *win) {
  const std::string stringPush = std::to_string(push);
  const std::string stringPull = std::to_string(pull);
  wattron(win, COLOR_PAIR(1));
  mvwprintw(win, 1, COLS - 10 - stringPull.size() - stringPush.size(), (" + " + stringPush).c_str());
  wattroff(win, COLOR_PAIR(1));
  wattron(win, COLOR_PAIR(2));
  wprintw(win, ("  - " + stringPull + " ").c_str());
  wattroff(win, COLOR_PAIR(2));
}

struct curses {
  curses() {
    // Win32a PDCurses -- make window resizable
    ttytype[0] = 25; // min  25 lines
    ttytype[1] = (unsigned char) 255; // max 255 lines
    ttytype[2] = 80; // min  80 columns
    ttytype[3] = (unsigned char) 255; // max 255 columns
    initscr();
    cbreak();
    keypad(stdscr, TRUE); // (required for resizing to work on Win32a ???)
    curs_set(0);
  }

  ~curses() { endwin(); }
};
