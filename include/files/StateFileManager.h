#pragma once
#include "FileManager.h"

class StateFileManager : public FileManager {
public:
    explicit StateFileManager(const std::string& filename);
};
