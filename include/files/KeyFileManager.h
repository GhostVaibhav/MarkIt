#pragma once
#include "FileManager.h"

class KeyFileManager : public FileManager {
 public:
  explicit KeyFileManager(const std::string& filename);
};
