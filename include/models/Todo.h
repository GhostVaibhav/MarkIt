#pragma once

#include <string>
#include <cstdint>
#include "json.hpp"

struct Todo {
    std::string id;
    std::string name;
    std::string desc;
    int32_t time;
    bool isComplete;

    static Todo create(const std::string& name, const std::string& desc);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Todo, id, name, desc, time, isComplete)
