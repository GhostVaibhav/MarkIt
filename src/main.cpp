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

// ------------------------------------------------------------------------
// ---------------------------HEADER FILES---------------------------------
// ------------------------------------------------------------------------

#include "cli.h" // For adding the CLI functionality
#include "cloudFunctions.h" // For using other Core Cloud functions
#include "fileHandlers.h" // For using file handler functions
#include "globalVariable.h" // For using PantryID
#include "json.hpp" // For using nlohmann::json

#include "curses.h" // For using Curses
#include "logger.h" // For using the logger
#include "toggles.h" // For including toggles

#include "backend.h"
#include "panels.h"

#include "spdlog/sinks/basic_file_sink.h" // For adding the logging functionality

#include <iostream>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define SystemOpenURL(url) system("start " url);
#elif __APPLE__
#define SystemOpenURL(url) system("open " url);
#elif __linux__
#define SystemOpenURL(url) system("xdg-open" url);
#else
#error "Unknown compiler"
#endif

// ------------------------------------------------------------------------
// ------------------MACROS, NAMESPACES AND DEFINITIONS--------------------
// ------------------------------------------------------------------------

void addColors() {
  logger->info("Adding colors");
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  init_color(COLOR_RED, 1000, 0, 0);
  init_pair(2, COLOR_RED, COLOR_BLACK);
  init_color(COLOR_BLUE, 0, 0, 1000);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_color(COLOR_MAGENTA, 750, 0, 650);
  init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
  init_color(COLOR_YELLOW, 1000, 750, 0);
  init_pair(5, COLOR_YELLOW, COLOR_BLACK);
  init_pair(6, COLOR_WHITE, COLOR_BLACK);
  logger->info("Colors added");
}

void setTitle() {
#ifdef _WIN32
  LPCSTR title = "MarkIt!";
  SetConsoleTitleA(title);
#else
  std::string title = "MarkIt!";
  std::cout << "\033]0;" << title << "\007";
#endif
}

// ------------------------------------------------------------------------
// ----------------------------MAIN FUNCTION-------------------------------
// ------------------------------------------------------------------------

int main(const int argc, char *argv[]) {
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);

  logger = spdlog::basic_logger_mt("basic_logger", "markit_logs.txt");
  logger->info("Logger initialized");
  setTitle();
  if (argc > 1) {
    std::vector<std::string> args(0);
    for (int i = 1; i < argc; i++)
      args.emplace_back(argv[i]);
#ifdef DEBUG
    if (args[0] == "--test" || args[0] == "-t" || args[0] == "test") {
      cli::test(args);
    }
#endif
    if (args[0] == "--version" || args[0] == "-v" || args[0] == "version") {
      cli::version(args);
    }
    if (args[0] == "--display" || args[0] == "-d" || args[0] == "display") {
      cli::display(args);
    }
    return 0;
  }
  initscr();
  cbreak();
  start_color();
  curs_set(0);
  keypad(stdscr, true);
  addColors();
  LoadingPanel("Reading key file");
  try {
    json temp = json::parse(_read_from_file(keyFile));
    pantryId = temp["key"];
  } catch (...) {
    logger->error("Could not read key file");
  }

  int loggedIn = -1;
  if (!exist(stateFile)) {
    logger->info("State file doesn't exist, getting login credentials");
    loggedIn = LoginPanel(&curUser);
  }

  LoadingPanel("Reading state file");
  if (loggedIn == -1) {
    try {
      json temp = json::parse(_read_from_file(stateFile));
      curUser = temp["userName"];
      curUserHash = temp["userHash"];
      cloudSave = getBucketDetails(curUser);
    } catch (...) {
      logger->debug("Current user doesn't exist");
      _delete_file(stateFile);
      loggedIn = LoginPanel(&curUser);
    }
  }

  try {
    localSave = json::parse(_read_from_file(storageFile));
  } catch (...) {
    _delete_file(storageFile);
    localSave = cloudSave;
    _write_to_file(localSave, storageFile);
  }
  if (IsCorrupted()) {
    logger->error("Data in the save file is corrupted");
    _delete_file(storageFile);
    localSave = cloudSave;
    _write_to_file(localSave, storageFile);
  }

  UpdatePP();

  WelcomePanel(loggedIn);
  MainMenuPanel();
  endwin();
}
