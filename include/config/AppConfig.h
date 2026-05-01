#pragma once
#include <string>

#include "toggles.h"
#include "utils/PathUtils.h"

struct AppConfig {
  int minWidth = 78;
  int minHeight = 20;
  std::string stateFile = PathUtils::getDataPath() + "/state.dat";
  std::string keyFile = PathUtils::getDataPath() + "/key.dat";
  std::string dbFile = PathUtils::getDataPath() + "/markit.db";
  std::string version = std::to_string(MARKIT_MAJOR_VERSION) + "." +
                        std::to_string(MARKIT_MINOR_VERSION);
};
