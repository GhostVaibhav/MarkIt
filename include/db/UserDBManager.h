#pragma once
#include "DBManager.h"
#include "User.h"
#include <vector>

class UserDBManager : public DBManager {
public:
    explicit UserDBManager(const std::string& dbPath);
    
    std::vector<User> getUsers();
    bool addUser(const User& user);
    bool removeUser(const User& user);
    bool updateUser(const User& user);
    bool existUser(const User& user);
};
