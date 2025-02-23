/*
 *      __  ___         __    ____ __  __
 *     /  |/  /__ _____/ /__ /  _// /_/ /
 *    / /|_/ / _ `/ __/  '_/_/ / / __/_/
 *   /_/  /_/\_,_/_/ /_/\_\/___/ \__(_)
 *
 *   MIT License
 *
 *   Copyright (c) 2021 Vaibhav Sharma
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

#include "globalVariable.h"

// ------------------------------------------------------------------------
// ---------------------------GLOBAL VARIABLES-----------------------------
// ------------------------------------------------------------------------
std::string curUser = ""; // For storing the current username
std::string curUserHash =
    ""; // For storing the current user password's SHA-256 hash
std::string PantryID =
    "dc8b010b-5dea-48f8-8ce9-21bf93b71aca"; // For storing the API key of the
                                            // Pantry
std::string storageFile =
    "data.dat"; // File name of the local storage file - DON'T CHANGE THIS!!
std::string stateFile =
    "state.dat"; // File name of the local state file - DON'T CHANGE THIS!!
std::string keyFile =
    "key.dat"; // File name of the local key file - DON'T CHANGE THIS!!
int push = 0;  // Global variable for keeping track of "push" requests
int pull = 0;  // Global variable for keeping track of "pull" requests
