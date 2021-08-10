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
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *   
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *   
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 */

// ------------------------------------------------------------------------
// ---------------------------HEADER FILES---------------------------------
// ------------------------------------------------------------------------

#include <iostream>    // For using Strings
#include <fstream>     // For using File operations
#include <algorithm>   // For using Standard Algorithms
#include <vector>      // For using std::vector
#include <curl/curl.h> // For using Curl
#include <json.hpp>    // For using nlohmann::json
#include <sha256.h>    // For using SHA-256 algorithm
#include <memory>      // For using unique pointer
#include <sys/stat.h>  // For checking if a file exists or not
#ifdef _WIN32
#include <cstdio>            // For using _popen() and _pclose()
#include <PDCurses/curses.h> // For using PDCurses on Windows platform
#else
#include <curses.h> // For using Ncurses on Unix-based platforms
#include <termios.h>
#endif

// ------------------------------------------------------------------------
// ------------------MACROS, NAMESPACES AND DEFINITIONS--------------------
// ------------------------------------------------------------------------

#ifndef JSON              // Checking if JSON is defined or not
nlohmann::json cloudSave; // Cloud save is loaded in the memory once the user signs in
nlohmann::json localSave; // Local save is also loaded in the memory once the user signs in
#endif
using json = nlohmann::json;                             // Using namespace for minimizing the write effort
#define minWidth 78                                      // Defining the minimum width of the window in pixels - DON'T CHANGE THIS!!
#define minHeight 20                                     // Defining the minimum height of the window in pixels - DON'T CHANGE THIS!!
#define BORDER(win) wborder(win, 0, 0, 0, 0, 0, 0, 0, 0) // Defining a macro for drawing a border around a border
std::string curUser = "";                                // For storing the current username
std::string curUserHash = "";                            // For storing the current user password's SHA-256 hash
std::string PantryID;                                    // For storing the API key of the Pantry
std::string storageFile = "data.dat";                    // File name of the local storage file - DON'T CHANGE THIS!!
std::string stateFile = "state.dat";                     // File name of the local state file - DON'T CHANGE THIS!!
std::string keyFile = "key.dat";                         // File name of the local key file - DON'T CHANGE THIS!!
int push = 0;                                            // Global variable for keeping track of "push" requests
int pull = 0;                                            // Global variable for keeping track of "pull" requests

// ------------------------------------------------------------------------
// ---------------------CORE STRUCTURE OF TODO USED------------------------
// ------------------------------------------------------------------------

struct todo
{
    std::string name;                                                   // Storing the name of Todo
    std::string desc;                                                   // Storing the description of Todo
    std::string time;                                                   // Automatically generating the time for a Todo
    bool isComplete;                                                    // Marking the Todo as "complete" or "not complete"
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(todo, name, desc, time, isComplete); // For serializing and deserializing JSON from Todo structure and vice-versa
};

// ------------------------------------------------------------------------
// ---------------------CORE FILE SERVICE FUNCTIONS------------------------
// ------------------------------------------------------------------------
// This section consists of three internal file modification functions:
// 1. _write_to_file() - For writing to the local file
// -----------------------------------------------------------------------
// | Returns: NOTHING                                                    |
// | Parameters:                                                         |
// | json object - The JSON object to be written to the file             |
// | std::string - The file name to be written to                        |
// -----------------------------------------------------------------------
// 2. _read_from_file() - For reading from the local file
// -------------------------------------------------------------------------------------
// | Returns: std::string - The JSON object read from the file in the form of a string |
// | Parameters: std::string - The file name to be read from                           |
// -------------------------------------------------------------------------------------
// 3. _delete_file() - For deleting the file's contents
// ----------------------------------------------------
// | Returns: NOTHING                                 |
// | Parameters: std::string - The file to be deleted |
// ----------------------------------------------------

void _write_to_file(json temp, std::string STORAGE_FILE = storageFile)
{
    std::ofstream f1(STORAGE_FILE);
    f1 << temp.dump();
    f1.close();
}

std::string _read_from_file(std::string STORAGE_FILE = storageFile)
{
    std::ifstream f1(STORAGE_FILE.c_str());
    std::string _read_string;
    if (!f1.is_open())
        _read_string = "File does not exist";
    else
        std::getline(f1, _read_string);
    f1.close();
    return _read_string;
}

void _delete_file(std::string STORAGE_FILE = storageFile)
{
    std::ofstream f1;
    f1.open(STORAGE_FILE, std::ofstream::out | std::ofstream::trunc);
    f1.close();
}

// ------------------------------------------------------------------------
// --------------------CORE CLOUD SERVICE FUNCTIONS------------------------
// ------------------------------------------------------------------------
// This section consists of functions related to API calling and saving the responses
// The functions are as follows:
// 1. updatePP() - Updates the push and pull counter on the top-right corner of the screen // todo: Fix this function
// -----------------------
// | Returns: NOTHING    |
// | Parameters: NOTHING |
// -----------------------
// 2. write_to_string() - For using it with Curl and writing response to a string
// --------------------------------------------------------
// | Returns: size_t (Determining the size of the string) |
// | Parameters:                                          |
// | void* - Storing the pointer to the text to append    |
// | size_t - Size of a character                         |
// | size_t - Count of a character                        |
// | void* - Main string to write                         |
// --------------------------------------------------------
// 3. getBucket() - Getting a bucket from an API call
// -----------------------------------------
// | Returns: bool - If the bucket exists  |
// | Parameters: std::string - Bucket name |
// -----------------------------------------
// 4. createBucket() - Creating a bucket through an API call
// --------------------------------------------
// | Returns: int - Returning a creation code |
// | Parameters: std::string - Bucket name    |
// --------------------------------------------
// 5. replaceBucket() - Replacing an existing bucket through an API call
// -----------------------------------------------------------------------
// | Returns: bool - If the replacement was successful                   |
// | Parameters: std::string, json - Bucket name, JSON object to replace |
// -----------------------------------------------------------------------
// 6. deleteBucket() - Deleting a bucket through an API call
// ------------------------------------------------
// | Returns: bool - If the delete was successful |
// | Parameters: std::string - Bucket name        |
// ------------------------------------------------
// 7. getBucketDetails() - Getting the bucket details (in the form of string) and serializing it to a JSON structure through an API call
// -------------------------------------------------
// | Returns: json - JSON object of bucket details |
// | Parameters: std::string - Bucket name         |
// -------------------------------------------------
// 8. appendBucket() - Appending to the end of a bucket through an API call (useful in pushing to the cloud)
// ----------------------------------------------------------------------
// | Returns: bool - If the append was successful                       |
// | Parameters: std::string, json - Bucket name, JSON object to append |
// ----------------------------------------------------------------------

void updatePP()
{
    int allChange = localSave["data"].size() - cloudSave["data"].size();
    if (allChange >= 0)
    {
        push = allChange;
        pull = 0;
    }
    else
    {
        push = 0;
        pull = -allChange;
    }
}

size_t write_to_string(void *ptr, size_t size, size_t count, void *stream)
{
    ((std::string *)stream)->append((char *)ptr, 0, size * count);
    return size * count;
}

bool getBucket(const std::string &bucketName)
{
    CURL *curl;
    CURLcode res;
    std::string resp;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        std::string url = "https://getpantry.cloud/apiv1/pantry/" + PantryID + "/basket/" + bucketName;
        curl_easy_setopt(curl, CURLOPT_URL, (url.c_str()));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        const char *data = "";
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
        if (res != 0)
            throw curl_easy_strerror(res);
    }
    curl_easy_cleanup(curl);
    if (resp.find("Could not get basket") != resp.npos)
        return false;
    else
        return true;
}

int createBucket(const std::string &bucketName)
{
    std::string resp;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        std::string url = "https://getpantry.cloud/apiv1/pantry/" + PantryID + "/basket/" + bucketName;
        curl_easy_setopt(curl, CURLOPT_URL, (url.c_str()));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        const char *data = "";
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    return (int)res;
}

bool replaceBucket(const std::string &bucketName, const json &bucket)
{
    if (!getBucket(bucketName))
        return false;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string resp;
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        std::string url = "https://getpantry.cloud/apiv1/pantry/" + PantryID + "/basket/" + bucketName;
        curl_easy_setopt(curl, CURLOPT_URL, (url.c_str()));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        std::string update = bucket.dump();
        const char *data = update.c_str();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    return true;
}

bool deleteBucket(const std::string &bucketName)
{
    CURL *curl;
    CURLcode res;
    std::string resp;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        std::string url = "https://getpantry.cloud/apiv1/pantry/" + PantryID + "/basket/" + bucketName;
        curl_easy_setopt(curl, CURLOPT_URL, (url.c_str()));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        const char *data = "";
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    if (resp.find("Could not delete") != std::string::npos)
        return false;
    else
        return true;
}

json getBucketDetails(const std::string &bucketName)
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    json temp;
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        std::string url = "https://getpantry.cloud/apiv1/pantry/" + PantryID + "/basket/" + bucketName;
        curl_easy_setopt(curl, CURLOPT_URL, (url.c_str()));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        const char *data = "";
        std::string result;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
        if (res != 0)
            throw curl_easy_strerror(res);
        temp = json::parse(result);
    }
    curl_easy_cleanup(curl);
    return temp;
}

bool appendBucket(const std::string &bucketName, const json &patch)
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string resp;
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        std::string url = "https://getpantry.cloud/apiv1/pantry/" + PantryID + "/basket/" + bucketName;
        curl_easy_setopt(curl, CURLOPT_URL, (url.c_str()));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        std::string update = patch.dump();
        const char *data = update.c_str();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    if (resp.find("does not exist") != resp.npos)
        return false;
    else
        return true;
}

// ------------------------------------------------------------------------
// ----------------------CORE WORKER FUNCTIONS-----------------------------
// ------------------------------------------------------------------------
// This section contains the second layer of functions which uses the core cloud and file functions.
// The functions are as below:
// 1. logo() - prints the logo at the specified location on the screen
// ------------------------------------------
// | Returns: NOTHING                       |
// | Parameters:                            |
// | WINDOW* - the window to print on       |
// | int - starting x - co-ordinate of logo |
// | int - starting y - co-ordinate of logo |
// ------------------------------------------
// 2. computeTime() - a lambda function which gives the current time in the form of a string
// ---------------------------------------------------------------
// | Returns: std::string - present time in the form of a string |
// | Parameters: NOTHING                                         |
// ---------------------------------------------------------------
// 3. resize_event() - resizes the terminal window
// -----------------------
// | Returns: NOTHING    |
// | Parameters: NOTHING |
// -----------------------
// 4. addTodo() - adds a todo to the todo list
// -----------------------------------------------------------
// | Returns: bool - If todo was successfully added          |
// | Parameters: WINDOW* - The terminal WINDOW to display on |
// -----------------------------------------------------------
// 5. loading() - displays a loading screen with a custom message
// --------------------------------------------------------------------------
// | Returns: NOTHING                                                       |
// | Parameters: std::string - String to be displayed on the loading screen |
// --------------------------------------------------------------------------
// 6. refreshCloudSave() - updates the cloud save, local save and update the push-pull counter
// -----------------------
// | Returns: NOTHING    |
// | Parameters: NOTHING |
// -----------------------
// 7. print_stats() - prints the current stats (i.e push and pull counter)
// ---------------------------------------------------------------------
// | Returns: NOTHING                                                  |
// | Parameters: WINDOW* - the terminal WINDOW to display the stats on |
// ---------------------------------------------------------------------
// 8. pullFromCloud() - pulls the todos from the cloud and updates the local save in accordance with the cloud save
// ------------------------------------------------------------------------
// | Returns: bool - If pull was successful                               |
// | Parameters: WINDOW* - the terminal window to display any information |
// ------------------------------------------------------------------------
// 9. pushToCloud() - pushes the todos to the cloud and updates the local save as well as the cloud save
// -----------------------------------------------------------------------------------------
// | Returns: NOTHING                                                                      |
// | Parameters: WINDOW* - the terminal window to display any information (loading screen) |
// -----------------------------------------------------------------------------------------
// 10. corruptedData() - checks if a file is corrupted
// ------------------------------------------------------
// | Returns: bool - return false, if file is corrupted |
// | Parameters: NOTHING                                |
// ------------------------------------------------------
// 11. deleteTodo() - deletes a todo from the todo list
// ---------------------------------------------------------
// | Returns: bool - return true, if todo is deleted       |
// | Parameters: int* - signifying the index to be deleted |
// ---------------------------------------------------------
// 12. exist() - checks if a file exists
// -----------------------------------------------
// | Returns: bool - return true, if file exists |
// | Parameters: std::string - file name         |
// -----------------------------------------------

void logo(WINDOW *win, int x = 0, int y = 1) noexcept
{
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
    mvwprintw(win, x + 4, y + 30, R"((_))");
    wattroff(win, COLOR_PAIR(1));
}

auto computeTime = [] () -> std::string {
    time_t lt;
    lt = time(NULL);
    struct tm *tempTime = localtime(&lt);
    return asctime(tempTime);
};

struct curses
{
    curses()
    {
        // Win32a PDCurses -- make window resizable
        ttytype[0] = 25;                 // min  25 lines
        ttytype[1] = (unsigned char)255; // max 255 lines
        ttytype[2] = 80;                 // min  80 columns
        ttytype[3] = (unsigned char)255; // max 255 columns
        initscr();
        cbreak();
        keypad(stdscr, TRUE); // (required for resizing to work on Win32a ???)
        curs_set(0);
    }

    ~curses()
    {
        endwin();
    }
};

void resize_event()
{
    int w, h;
    resize_term(0, 0);
    getmaxyx(stdscr, h, w);
    clear();
    BORDER(stdscr);
}

bool addTodo(WINDOW *win)
{
    wclear(win);
    wrefresh(win);
    box(win, 0, 0);
    todo t;
    char name[64];
    char desc[256];
    mvwprintw(win, getmaxy(win) / 2 - 1, 10, "Enter title: ");
    mvwgetnstr(win, getmaxy(win) / 2 - 1, 24, name, 63);
    box(win, 0, 0);
    wrefresh(win);
    mvwprintw(win, getmaxy(win) / 2, 10, "Enter description: ");
    mvwgetnstr(win, getmaxy(win) / 2, 30, desc, 255);
    t.name = name;
    t.desc = desc;
    if (!t.name.size() || !t.desc.size())
        return false;
    t.time = computeTime();
    t.isComplete = false;
    std::vector<todo> temp;
    temp.push_back(t);
    localSave["data"] = temp;
    int number = localSave["number"];
    number++;
    localSave["number"] = number;
    _write_to_file(localSave);
    updatePP();
    return true;
}

void loading(std::string loadText)
{
    clear();
    int part = (getmaxx(stdscr) - 36) / 2;
    int half = ((getmaxy(stdscr) - 4) / 2) - 1;
    refresh();
    wrefresh(stdscr);
    logo(stdscr, half, part);
    mvwprintw(stdscr, getmaxy(stdscr) - 2, (getmaxx(stdscr) - loadText.size()) / 2, loadText.c_str());
    wrefresh(stdscr);
    refresh();
}

void refreshCloudSave()
{
    loading("Refreshing");
    cloudSave = getBucketDetails(curUser);
    localSave = json::parse(_read_from_file());
    updatePP();
}

void print_stats(WINDOW *win)
{
    std::string stringPush = std::to_string(push);
    std::string stringPull = std::to_string(pull);
    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 1, COLS - 10 - stringPull.size() - stringPush.size(), (" + " + stringPush).c_str());
    wattroff(win, COLOR_PAIR(1));
    wattron(win, COLOR_PAIR(2));
    wprintw(win, ("  - " + stringPull + " ").c_str());
    wattroff(win, COLOR_PAIR(2));
}

bool pullFromCloud(WINDOW *win)
{
    loading("Pulling from cloud");
    cloudSave = getBucketDetails(curUser);
    if (localSave == cloudSave)
        return true;
    json j = localSave.diff(localSave, cloudSave);
    if (!j.is_null())
        return false;
    localSave = cloudSave;
    return true;
}

void pushToCloud(WINDOW *win)
{
    loading("Pushing to cloud");
    cloudSave = getBucketDetails(curUser);
    localSave = json::parse(_read_from_file());
    if (localSave == cloudSave || replaceBucket(curUser, localSave))
    {
        updatePP();
        print_stats(win);
        return;
    }
}

bool corruptedData()
{
    if (localSave["number"] != localSave["data"].size() || !localSave["data"].is_array() || localSave["hash"] != curUserHash)
    {
        return true;
    }
    try
    {
        std::vector<todo> t = localSave["data"];
        for (todo a : t)
            if (!a.name.size() || !a.desc.size() || !a.time.size())
                return true;
    }
    catch (json::out_of_range &e)
    {
        return true;
    }
    return false;
}

bool deleteTodo(int *selection)
{
    std::vector<todo> temp = localSave["data"];
    if (temp.size() < *selection)
        return false;
    try
    {
        temp.erase(temp.begin() + (*selection));
        localSave["data"] = temp;
        int number = localSave["number"];
        number--;
        localSave["number"] = number;
        _write_to_file(localSave);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

inline bool exist(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

// ------------------------------------------------------------------------
// -----------------------CORE DISPLAY FUNCTIONS---------------------------
// ------------------------------------------------------------------------
// These functions are used to display in the terminal window.
// The functions are as below:
// 1. printCenter() - takes a vector of strings and prints them in the center with a focus on selection
// ---------------------------------------------------------------------
// | Returns: NOTHING                                                  |
// | Parameters:                                                       |
// | int* - the selection pointer                                      |
// | std::vector<std::string> - vector of strings to display on screen |
// | WINDOW* - the terminal WINDOW to display on                       |
// ---------------------------------------------------------------------
// 2. menu() - displays a custom menu on the basis of a vector of strings
// ----------------------------------------------------------------------------
// | Returns: int - returns the choice made                                   |
// | Parameters: std::vector<std::string> - vector of strings to be displayed |
// ----------------------------------------------------------------------------
// 3. main_menu() - displays the main menu
// -----------------------
// | Returns: NOTHING    |
// | Parameters: NOTHING |
// -----------------------
// 4. login() - displays the login screen
// --------------------------------------------------------
// | Returns: int - returns a login code                  |
// | Parameters: std::string* - returning the bucket name |
// --------------------------------------------------------
// 5. welcome() - displays the welcome screen
// -----------------------------------------------------------------------------
// | Returns: NOTHING                                                          |
// | Parameters: int - display a welcome screen on the basis of the login code |
// -----------------------------------------------------------------------------
// 6. add_colors() - adds the color pairs to the curses setup
// -----------------------
// | Returns: NOTHING    |
// | Parameters: NOTHING |
// -----------------------
// 7. set_title() - sets the title of the terminal window
// -----------------------
// | Returns: NOTHING    |
// | Parameters: NOTHING |
// -----------------------
// 8. generateKey() - prompts the user to enter his/her API key manually - not prompted if the key file is valid
// -----------------------
// | Returns: NOTHING    |
// | Parameters: NOTHING |
// -----------------------

void printCenter(int *selected, std::vector<std::string> a, WINDOW *win)
{
    int maxSize = (*max_element(a.begin(), a.end())).size();
    for (int i = 0; i < a.size(); i++)
    {
        if (i != *selected)
            mvwprintw(win, (getmaxy(win) / 2) + (i - (a.size() / 2)), (getmaxx(win) / 2) - maxSize, a[i].c_str());
        else
        {
            wattron(win, COLOR_PAIR(1));
            mvwprintw(win, (getmaxy(win) / 2) + (i - (a.size() / 2)), (getmaxx(win) / 2) - maxSize, a[i].c_str());
            wattroff(win, COLOR_PAIR(1));
        }
    }
}

int menu(std::vector<std::string> a)
{
    WINDOW *title = newwin(10, getmaxx(stdscr) - 2, 1, 1);
    WINDOW *menu = newwin(getmaxy(stdscr) - 12, getmaxx(stdscr) - 2, 11, 1);
    int pointerIndex = 0;
    int c;
    keypad(stdscr, true);
    refresh();
    while (1)
    {
        curs_set(0);
        resize_event();
#ifdef _WIN32
        resize_window(title, 10, getmaxx(stdscr) - 2);
        resize_window(menu, getmaxy(stdscr) - 12, getmaxx(stdscr) - 2);
#else
        wresize(title, 10, getmaxx(stdscr) - 2);
        wresize(menu, getmaxy(stdscr) - 12, getmaxx(stdscr) - 2);
#endif
        if (getmaxx(stdscr) >= minWidth && getmaxy(stdscr) >= minHeight)
        {
            int part = (getmaxx(title) - 81) / 4;
            int half = 1;
            if (part <= 0)
                part = 1;
            wclear(title);
            wclear(menu);
            wrefresh(title);
            wrefresh(menu);
            box(title, 0, 0);
            box(menu, 0, 0);
            logo(title, half, part);
            mvwprintw(title, 3, 3 * part + 25, ("Username: " + curUser).c_str());
            mvwprintw(title, 5, 3 * part + 25, ("Pantry ID: " + PantryID).c_str());
            printCenter(&pointerIndex, a, menu);
            refresh();
            print_stats(stdscr);
            wrefresh(title);
            wrefresh(menu);
        }
        else
        {
            if(getmaxx(stdscr) < minWidth) mvprintw(LINES / 2, (COLS - 35) / 2, "Please increase your window's width");
            else    mvprintw(LINES / 2, (COLS - 36) / 2, "Please increase your window's height");
        }
        c = getch();
        switch (c)
        {
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

void main_menu()
{
    curs_set(0);
    bool changesMade = false;
    WINDOW *todoUserName = newwin(10, getmaxx(stdscr) - 2, 1, 1);
    WINDOW *todoWindow = newwin(getmaxy(stdscr) - 12, getmaxx(stdscr) - 2, 11, 1);
    WINDOW *todoBody = newwin(getmaxy(todoWindow) - 4, getmaxx(todoWindow) - 1, getmaxy(todoUserName) + 4, 1);
    int c = KEY_RESIZE, pointerIndex = 0, moveFactor = 0, bottomT = 0, topT = getmaxy(todoBody);
    keypad(todoWindow, true);
    while (1)
    {
        std::vector<todo> temp = localSave["data"];
        if (changesMade)
        {
            refreshCloudSave();
            changesMade = false;
        }
        noecho();
        updatePP();
        curs_set(0);
        if (c == KEY_RESIZE)
        {
            resize_event();
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
        if (getmaxx(stdscr) >= minWidth && getmaxy(stdscr) >= minHeight)
        {
            int part = (getmaxx(todoUserName) - 81) / 4;
            int half = 1;
            if (part <= 0)
                part = 1;
            if (c == KEY_RESIZE)
            {
                wclear(todoWindow);
                wclear(todoUserName);
                wrefresh(todoWindow);
                box(todoWindow, 0, 0);
                box(todoUserName, 0, 0);
            }
            wclear(todoBody);
            wrefresh(todoBody);
            refresh();
            logo(todoUserName, half, part);
            mvwprintw(todoUserName, 3, 3 * part + 25, ("Username: " + curUser).c_str());
            mvwprintw(todoUserName, 5, 3 * part + 25, ("Pantry ID: " + PantryID).c_str());
            int tabDiv = (getmaxx(todoWindow) - 2) / 3;
            mvwvline(todoWindow, 1, tabDiv, 0, 1);
            mvwvline(todoWindow, 1, 2 * tabDiv, 0, 1);
            mvwhline(todoWindow, 2, 1, 0, getmaxx(todoWindow) - 2);
            mvwprintw(todoWindow, 1, (tabDiv - 4) / 2, "Name");
            mvwprintw(todoWindow, 1, ((3 * tabDiv - 12) / 2) + 1, "Description");
            mvwprintw(todoWindow, 1, ((5 * tabDiv - 13) / 2) + 2, "Created Time");
            BORDER(todoWindow);
            for (int i = 0; i < temp.size(); i++)
            {
                if (pointerIndex == i)
                {
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
                if (pointerIndex == i)
                {
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
            print_stats(stdscr);
            wrefresh(todoWindow);
            wrefresh(todoUserName);
            wrefresh(todoBody);
        }
        else
        {
            if(getmaxx(stdscr) < minWidth) mvprintw(LINES / 2, (COLS - 35) / 2, "Please increase your window's width");
            else    mvprintw(LINES / 2, (COLS - 36) / 2, "Please increase your window's height");
        }
        c = getch();
        switch (c)
        {
        case KEY_UP:
            pointerIndex--;
            if (pointerIndex < 0)
                pointerIndex = 0;
            if (pointerIndex < bottomT)
            {
                bottomT--;
                topT--;
                moveFactor++;
            }
            break;
        case KEY_DOWN:
            pointerIndex++;
            if (pointerIndex >= temp.size())
                pointerIndex = temp.size() - 1;
            if (pointerIndex >= topT)
            {
                bottomT++;
                topT++;
                moveFactor--;
            }
            break;
        case KEY_F(5):
        {
            refresh();
            echo();
            int choice = menu({"1. Add a todo", "2. Push all changes", "3. Pull from the cloud", "4. Refresh"});
            if (choice == 0)
            {
                bool c = addTodo(todoWindow);
                while (!c)
                    c = addTodo(todoWindow);
            }
            else if (choice == 1)
            {
                pushToCloud(stdscr);
                changesMade = true;
            }
            else if (choice == 2)
            {
                pullFromCloud(stdscr);
                changesMade = true;
            }
            else if (choice == 3)
                refreshCloudSave();
        }
            noecho();
            c = KEY_RESIZE;
            break;
        case KEY_F(6):
            return;
        case '\n':
        {
            refresh();
            echo();
            int choice = menu({"1. Delete", "2. Mark/Unmark"});
            if (choice == 0)
            {
                if (deleteTodo(&pointerIndex))
                {
                    loading("Todo deleted");
                }
                else
                {
                    loading("Todo could not be deleted");
                }
            }
            else if (choice == 1)
            {
                temp.at(pointerIndex).isComplete = !temp.at(pointerIndex).isComplete;
                localSave["data"] = temp;
                _write_to_file(localSave);
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

int login(std::string *bucket)
{
    curs_set(0);
    int part = (getmaxy(stdscr) - 18) / 4;
    char userName[32];
    char password[64];
    WINDOW *title = newwin(8, getmaxx(stdscr), part, 0);
    WINDOW *userNameWindow = newwin(5, getmaxx(stdscr) - 20, 2 * part + 8, 10);
    WINDOW *passwordWindow = newwin(5, getmaxx(stdscr) - 20, 3 * part + 13, 10);
    WINDOW *information = newwin(3, getmaxx(stdscr), getmaxy(stdscr) - 3, 0);
    WINDOW *wrongPassword = newwin(3, getmaxx(stdscr), 10, 0);
    while (1)
    {
        curs_set(0);
        clear();
        resize_event();
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
        part = (getmaxy(stdscr) - 18) / 4;
        if (getmaxx(stdscr) >= minWidth && getmaxy(stdscr) >= minHeight)
        {
            wclear(userNameWindow);
            wclear(passwordWindow);
            wclear(title);
            wclear(information);
            wrefresh(userNameWindow);
            wrefresh(passwordWindow);
            wborder(stdscr, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
            BORDER(userNameWindow);
            BORDER(passwordWindow);
            int part = (getmaxx(title) - 34) / 2;
            int half = 0;
            logo(title, half, part);
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
        }
        else
        {
            if(getmaxx(stdscr) < minWidth) mvprintw(LINES / 2, (COLS - 35) / 2, "Please increase your window's width");
            else    mvprintw(LINES / 2, (COLS - 36) / 2, "Please increase your window's height");
        }
        std::string userNameString = userName;
        std::string passwordString = password;
        passwordString = sha256(passwordString);
        loading("Generating keys...");
        loading("Loading bucket details...");
        try
        {
            if (getBucket(userNameString))
            {
                cloudSave = getBucketDetails(userNameString);
                if (cloudSave["hash"] == passwordString)
                {
                    clear();
                    *bucket = userName;
                    curUserHash = passwordString;
                    json temp;
                    temp["userName"] = userName;
                    temp["userHash"] = passwordString;
                    _write_to_file(temp, stateFile);
                    return 2;
                }
                else
                {
                    wattron(wrongPassword, COLOR_PAIR(2));
                    mvwprintw(wrongPassword, 2, (getmaxx(wrongPassword) - 15) / 2, "Wrong Password");
                    wattroff(wrongPassword, COLOR_PAIR(2));
                }
            }
            else
            {
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
        }
        catch (const char *err)
        {
            std::string a = err;
            wattron(wrongPassword, COLOR_PAIR(2));
            mvwprintw(wrongPassword, 2, (getmaxx(wrongPassword) - a.size()) / 2, a.c_str());
            wattroff(wrongPassword, COLOR_PAIR(2));
        }
    }
}

void welcome(int code)
{
    if (code == -1)
        return;
    WINDOW *loading = newwin(0, 0, 0, 0);
    clear();
    int part = (getmaxx(stdscr) - 36) / 2;
    int half = ((getmaxy(stdscr) - 4) / 2) - 1;
    refresh();
    wrefresh(loading);
    logo(loading, half, part);
    if (code == 1)
        mvwprintw(loading, getmaxy(stdscr) - 2, (getmaxx(stdscr) - 10 - curUser.size()) / 2, ("Welcome, " + curUser).c_str());
    else if (code == 2)
        mvwprintw(loading, getmaxy(stdscr) - 2, (getmaxx(stdscr) - 15 - curUser.size()) / 2, ("Welcome back, " + curUser).c_str());
    wrefresh(loading);
    refresh();
    wgetch(loading);
}

void add_colors()
{
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
}

void set_title()
{
#ifdef _WIN32
    LPCSTR title = "MarkIt!";
    SetConsoleTitleA(title);
#else
    std::string title = "MarkIt!";
    std::cout << "\033]0;" << title << "\007";
#endif
}

bool generateKey()
{
    curs_set(0);
    int part = (getmaxy(stdscr) - 18) / 4;
    std::string key;
    char keyC[36];
    WINDOW *title = newwin(8, getmaxx(stdscr), part, 0);
    WINDOW *keyWindow = newwin(5, getmaxx(stdscr) - 20, 2 * part + 8, 10);
    WINDOW *infobox = newwin(5, getmaxx(stdscr) - 20, 3 * part + 13, 10);
    WINDOW *information = newwin(3, getmaxx(stdscr), getmaxy(stdscr) - 3, 0);
    while (1)
    {
        curs_set(0);
        clear();
        resize_event();
#ifdef _WIN32
        resize_window(keyWindow, 5, getmaxx(stdscr) - 20);
        resize_window(infobox, 5, getmaxx(stdscr) - 20);
        resize_window(title, 8, getmaxx(stdscr));
        resize_window(information, 3, getmaxx(stdscr));
#else
        wresize(keyWindow, 5, getmaxx(stdscr) - 20);
        wresize(infobox, 5, getmaxx(stdscr) - 20);
        wresize(title, 8, getmaxx(stdscr));
        wresize(information, 3, getmaxx(stdscr));
#endif
        part = (getmaxy(stdscr) - 18) / 4;
        if (getmaxx(stdscr) >= minWidth && getmaxy(stdscr) >= minHeight)
        {
            wclear(keyWindow);
            wclear(infobox);
            wclear(title);
            wclear(information);
            wrefresh(keyWindow);
            wrefresh(infobox);
            wborder(stdscr, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
            BORDER(keyWindow);
            BORDER(infobox);
            int part = (getmaxx(title) - 34) / 2;
            int half = 0;
            logo(title, half, part);
            mvwprintw(information, 1, (getmaxx(information) - 26) / 2, "Don't resize this window!");
            mvwprintw(keyWindow, getmaxy(keyWindow) / 2, 5, "API Key: ");
            mvwprintw(infobox, getmaxy(infobox) / 2, (getmaxx(infobox) - 76) / 2, "Get a free API key by visiting getpantry.cloud for storing your data online");
            wrefresh(title);
            wrefresh(information);
            wrefresh(infobox);
            wbkgd(keyWindow, COLOR_PAIR(1));
            wgetnstr(keyWindow, keyC, 36);
            wbkgd(keyWindow, COLOR_PAIR(6));
            BORDER(keyWindow);
        }
        else
        {
            if(getmaxx(stdscr) < minWidth) mvprintw(LINES / 2, (COLS - 35) / 2, "Please increase your window's width");
            else    mvprintw(LINES / 2, (COLS - 36) / 2, "Please increase your window's height");
            wgetch(stdscr);
            return false;
        }
        key = keyC;
        json temp;
        temp["key"] = key;
        if (key.length() == 36)
        {
            _write_to_file(temp, keyFile);
            PantryID = key;
        }
        break;
    }
    return true;
}

// ------------------------------------------------------------------------
// ----------------------------MAIN FUNCTION-------------------------------
// ------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
    if (argc > 1)
    {
        std::string arg = argv[1];
        if (arg == "--test")
        {
            std::cout << "Starting test mode (only for dev-builds)" << std::endl;
            std::cout << "Only call it when you know what you are doing" << std::endl;
            return 0;
        }
    }
    set_title();
    initscr();
    start_color();
    curs_set(0);
    keypad(stdscr, true);
    add_colors();
    if (!exist(keyFile) || _read_from_file(keyFile) == "")
    {
        bool valid = generateKey();
        while(!valid)   valid = generateKey();
    }
    loading("Reading key file");
    try
    {
        json temp = json::parse(_read_from_file(keyFile));
        PantryID = temp["key"];
    }
    catch (...)
    {
        bool valid = generateKey();
        while(!valid)   valid = generateKey();
    }
    int loggedIn = -1;
    if (!exist(stateFile))
    {
        loggedIn = login(&curUser);
    }
    loading("Reading state file");
    if(loggedIn == -1)
    {
        try
        {
            json temp = json::parse(_read_from_file(stateFile));
            curUser = temp["userName"];
            cloudSave = getBucketDetails(curUser);
        }
        catch(...)
        {
            _delete_file(stateFile);
            loggedIn = login(&curUser);
        }
    }
    try
    {
        localSave = json::parse(_read_from_file());
    }
    catch (...)
    {
        _delete_file();
        localSave = cloudSave;
        _write_to_file(localSave);
    }
    if (corruptedData())
    {
        _delete_file();
        localSave = cloudSave;
        _write_to_file(localSave);
    }
    updatePP();
    welcome(loggedIn);
    main_menu();
    endwin();
    return 0;
}