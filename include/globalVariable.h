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

#pragma once

#include "json.hpp"
#include "toggles.h"

#include <string>

extern std::string pantryId; // For storing the Pantry ID unique to the user
extern std::string curUser; // For storing the current username
extern std::string curUserHash; // For storing the current user password's SHA-256 hash
extern std::string storageFile; // File name of the local storage file - DON'T CHANGE THIS!!
extern std::string stateFile; // File name of the local state file - DON'T CHANGE THIS!!
extern std::string keyFile; // File name of the local key file - DON'T CHANGE THIS!!
extern int push; // Global variable for keeping track of "push" requests
extern int pull; // Global variable for keeping track of "pull" requests
extern nlohmann::json cloudSave;
extern nlohmann::json localSave;
#define minWidth 78 // Defining the minimum width of the window in pixels - DON'T CHANGE THIS!!
#define minHeight                                                                                                      \
  20 // Defining the minimum height of the window in pixels - DON'T CHANGE
     // THIS!!
#define BORDER(win) wborder(win, 0, 0, 0, 0, 0, 0, 0, 0) // Defining a macro for drawing a border around a border
#define APP_VERSION                                                                                                    \
  std::to_string(MARKIT_MAJOR_VERSION) + "." + std::to_string(MARKIT_MINOR_VERSION) // Defining the application version
