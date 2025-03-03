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

#include "backend.h"
#include "cloudFunctions.h"
#include "fileHandlers.h"
#include "globalVariable.h"
#include "logger.h"
#include "panels.h"
#include "sha256.h"
#include "structure.h"

bool PullFromCloud() {
  LoadingPanel("Pulling from cloud");
  cloudSave = getBucketDetails(curUser);
  if (localSave == cloudSave) {
    logger->info("Local and cloud saves are same, therefore not pulling");

    return true;
  }
  json j = nlohmann::json::diff(localSave, cloudSave);
  if (!j.is_null()) {
    logger->info("Pull patch exists: " + j.dump());

    localSave = cloudSave;
    UpdatePP();
    return false;
  }
  return true;
}

void PushToCloud(WINDOW *win) {
  LoadingPanel("Pushing to cloud");
  cloudSave = getBucketDetails(curUser);
  localSave = json::parse(_read_from_file(storageFile));
  if (localSave == cloudSave || replaceBucket(curUser, localSave)) {
    UpdatePP();
    StatsPanel(win);
    return;
  }
}

void RefreshCloudSave() {
  LoadingPanel("Refreshing");
  cloudSave = getBucketDetails(curUser);
  localSave = json::parse(_read_from_file(storageFile));
  UpdatePP();
}

void UpdatePP() {
  const int allChange = localSave["data"].size() - cloudSave["data"].size();
  if (allChange >= 0) {
    push = allChange;
    pull = 0;
  } else {
    push = 0;
    pull = -allChange;
  }
  logger->info("Updated push and pull counters, push: " + std::to_string(push) + " pull: " + std::to_string(pull) +
               " allChange: " + std::to_string(allChange));
}


void GetAPIKey(const std::string &userName) {
  // CURL *curl = curl_easy_init();
  // json key;
  // if (curl) {
  //   curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
  //   const std::string url = "https://markit-backend.herokuapp.com/user=" + userName;
  //   curl_easy_setopt(curl, CURLOPT_URL, (url.c_str()));
  //   curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  //   curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
  //   struct curl_slist *headers = nullptr;
  //   headers = curl_slist_append(headers, "Content-Type: application/json");
  //   curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  //   const auto data = "";
  //   std::string result;
  //   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
  //   curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
  //   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
  //   curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
  //   curl_easy_perform(curl);
  // }
  // curl_easy_cleanup(curl);
  // key["key"] = PantryID;
  // if (!key["key"].empty() || key["error"].empty()) {
  //   PantryID = key["key"];
  //   return true;
  // }
  // return false;
  pantryId = "local-";
  const std::string hash = sha256(std::to_string(time(nullptr)) + userName);
  pantryId += hash;
}

auto computeTime = []() -> std::string {
  time_t lt;
  lt = time(nullptr);
  const struct tm *tempTime = localtime(&lt);
  return asctime(tempTime);
};


bool IsCorrupted() {
  if (std::stoi(localSave["number"].dump()) != localSave["data"].size() || !localSave["data"].is_array() ||
      localSave["hash"] != curUserHash) {
    logger->error(std::stoi(localSave["number"].dump()));
    logger->error(localSave["data"].size());
    logger->error(localSave["data"].is_array());
    logger->error(localSave["hash"]);
    logger->error(curUserHash);

    return true;
  }
  try {
    const std::vector<todo> t = localSave["data"];
    for (const todo &a: t)
      if (a.name.empty() || a.desc.empty() || a.time.empty())
        return true;
  } catch (json::out_of_range &) {
    return true;
  }
  return false;
}

bool DeleteTodo(const unsigned int *selection) {
  std::vector<todo> temp = localSave["data"];
  if (temp.size() < *selection)
    return false;
  try {
    temp.erase(temp.begin() + (*selection));
    localSave["data"] = temp;
    int number = localSave["number"];
    number--;
    localSave["number"] = number;
    _write_to_file(localSave, storageFile);
  } catch (...) {
    return false;
  }
  return true;
}


bool AddTodo(WINDOW *win) {
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
  if (t.name.empty() || t.desc.empty())
    return false;
  t.time = computeTime();
  t.isComplete = false;
  std::vector<todo> temp = localSave["data"];
  temp.push_back(t);
  localSave["data"] = temp;
  int number = localSave["number"];
  number++;
  localSave["number"] = number;
  _write_to_file(localSave, storageFile);
  UpdatePP();
  return true;
}
