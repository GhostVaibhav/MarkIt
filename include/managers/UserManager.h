#pragma once
#include <optional>
#include <string>
#include <vector>

#include "StateFileManager.h"
#include "User.h"
#include "UserDBManager.h"

class UserManager {
 public:
  UserManager(const std::string& dbPath, const std::string& stateFilePath);

  bool addUser(const User& user);
  bool removeUser(const User& user);
  bool updateUser(const User& user);
  std::vector<User> getAllUsers();

  void setCurrentUser(const User& user);
  std::optional<User> getCurrentUser() const;

  bool saveSession();
  bool loadSession();
  bool clearSession();

 private:
  UserDBManager userDBManager;
  StateFileManager stateFileManager;
  std::optional<User> currentUser;
};
