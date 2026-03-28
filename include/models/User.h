#pragma once

#include <string>
#include "json.hpp"

struct User {
    std::string id;
    std::string name;
    std::string password;
    std::string pantryId;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(User, id, name, password, pantryId)
