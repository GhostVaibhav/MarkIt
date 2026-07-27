#include "UserManager.h"

#include <spdlog/spdlog.h>

UserManager::UserManager(const std::string& dbPath,
                         const std::string& stateFilePath)
    : userDBManager(dbPath), stateFileManager(stateFilePath) {}

bool UserManager::addUser(const User& user) {
  if (userDBManager.existUser(user)) {
    spdlog::warn("UserManager: Attempted to add duplicate user '{}'",
                 user.name);
    return false;
  }
  spdlog::info("UserManager: Adding new user '{}'", user.name);
  return userDBManager.addUser(user);
}

bool UserManager::removeUser(const User& user) {
  bool dbRemoved = userDBManager.removeUser(user);
  if (dbRemoved && currentUser && currentUser->id == user.id) {
    spdlog::info("UserManager: Removed active user '{}', clearing session",
                 user.name);
    clearSession();
  }
  return dbRemoved;
}

bool UserManager::updateUser(const User& user) {
  bool dbUpdated = userDBManager.updateUser(user);
  if (dbUpdated && currentUser && currentUser->id == user.id) {
    spdlog::info("UserManager: Updated active user '{}'", user.name);
    currentUser = user;
  }
  return dbUpdated;
}

std::vector<User> UserManager::getAllUsers() {
  return userDBManager.getUsers();
}

void UserManager::setCurrentUser(const User& user) {
  spdlog::info("UserManager: Setting current user to '{}'", user.name);
  currentUser = user;
}

std::optional<User> UserManager::getCurrentUser() const { return currentUser; }

bool UserManager::saveSession() {
  if (!currentUser) return false;
  nlohmann::json j;
  j["id"] = currentUser->id;
  bool result = stateFileManager.writeFile(j.dump());
  spdlog::info("UserManager: Saved session for user '{}', result: {}",
               currentUser->name, result);
  return result;
}

bool UserManager::loadSession() {
  auto content = stateFileManager.readFile();
  if (!content || content->empty()) return false;

  try {
    nlohmann::json j = nlohmann::json::parse(*content);
    if (!j.contains("id")) return false;
    std::string sid = j["id"].get<std::string>();
    for (const auto& u : getAllUsers()) {
      if (u.id == sid) {
        spdlog::info("UserManager: Successfully loaded session for user '{}'",
                     u.name);
        currentUser = u;
        return true;
      }
    }
    spdlog::warn("UserManager: Load session failed, user ID {} not found", sid);
    return false;
  } catch (...) {
    return false;
  }
}

bool UserManager::clearSession() {
  spdlog::info("UserManager: Clearing active session.");
  currentUser = std::nullopt;
  return stateFileManager.deleteFile();
}

void UserManager::saveRemoteCache(const std::string& userId, const std::string& cacheData) {
  userDBManager.saveRemoteCache(userId, cacheData);
}

std::string UserManager::getRemoteCache(const std::string& userId) {
  return userDBManager.getRemoteCache(userId);
}
