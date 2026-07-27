#include "UserDBManager.h"

#include <spdlog/spdlog.h>

UserDBManager::UserDBManager(const std::string& dbPath) : DBManager(dbPath) {
  db.exec(
      "CREATE TABLE IF NOT EXISTS users (id TEXT PRIMARY KEY, name TEXT, "
      "password TEXT, pantryId TEXT)");
  db.exec(
      "CREATE TABLE IF NOT EXISTS remote_cache (userId TEXT PRIMARY KEY, data TEXT)");
}

std::vector<User> UserDBManager::getUsers() {
  std::vector<User> users;
  SQLite::Statement query(db, "SELECT id, name, password, pantryId FROM users");
  while (query.executeStep()) {
    User u;
    u.id = query.getColumn(0).getString();
    u.name = query.getColumn(1).getString();
    u.password = query.getColumn(2).getString();

    if (query.getColumn(3).isText()) {
      u.pantryId = query.getColumn(3).getString();
    } else {
      u.pantryId = "";
    }
    users.push_back(u);
  }
  spdlog::info("UserDBManager: Fetched {} users from DB.", users.size());
  return users;
}

bool UserDBManager::addUser(const User& user) {
  try {
    SQLite::Statement query(
        db,
        "INSERT INTO users (id, name, password, pantryId) VALUES (?, ?, ?, ?)");
    query.bind(1, user.id);
    query.bind(2, user.name);
    query.bind(3, user.password);
    query.bind(4, user.pantryId);
    query.exec();
    spdlog::info("UserDBManager: Successfully added user {}", user.name);
    return true;
  } catch (const std::exception& e) {
    spdlog::error("UserDBManager: Error adding user {}: {}", user.name,
                  e.what());
    return false;
  }
}

bool UserDBManager::updateUser(const User& user) {
  try {
    SQLite::Statement query(
        db,
        "UPDATE users SET name = ?, password = ?, pantryId = ? WHERE id = ?");
    query.bind(1, user.name);
    query.bind(2, user.password);
    query.bind(3, user.pantryId);
    query.bind(4, user.id);
    query.exec();
    spdlog::info("UserDBManager: Successfully updated user {}", user.name);
    return true;
  } catch (const std::exception& e) {
    spdlog::error("UserDBManager: Error updating user {}: {}", user.name,
                  e.what());
    return false;
  }
}

bool UserDBManager::removeUser(const User& user) {
  try {
    SQLite::Statement query(db, "DELETE FROM users WHERE id = ?");
    query.bind(1, user.id);
    query.exec();
    spdlog::info("UserDBManager: Successfully removed user {}", user.id);
    return true;
  } catch (const std::exception& e) {
    spdlog::error("UserDBManager: Error removing user {}: {}", user.id,
                  e.what());
    return false;
  }
}

bool UserDBManager::existUser(const User& user) {
  SQLite::Statement query(db, "SELECT COUNT(*) FROM users WHERE name = ?");
  query.bind(1, user.name);
  query.executeStep();
  bool exists = query.getColumn(0).getInt() > 0;
  spdlog::info("UserDBManager: Checked existence for user {}, result: {}",
               user.name, exists);
  return exists;
}

void UserDBManager::saveRemoteCache(const std::string& userId, const std::string& cacheData) {
  try {
    SQLite::Statement query(db, "INSERT OR REPLACE INTO remote_cache (userId, data) VALUES (?, ?)");
    query.bind(1, userId);
    query.bind(2, cacheData);
    query.exec();
    spdlog::info("UserDBManager: Successfully saved remote cache for user {}", userId);
  } catch (const std::exception& e) {
    spdlog::error("UserDBManager: Error saving remote cache for user {}: {}", userId, e.what());
  }
}

std::string UserDBManager::getRemoteCache(const std::string& userId) {
  try {
    SQLite::Statement query(db, "SELECT data FROM remote_cache WHERE userId = ?");
    query.bind(1, userId);
    if (query.executeStep()) {
      return query.getColumn(0).getString();
    }
  } catch (const std::exception& e) {
    spdlog::error("UserDBManager: Error fetching remote cache for user {}: {}", userId, e.what());
  }
  return "";
}
