#pragma once
#include <string>

#include "toggles.h"

struct AppConfig {
  int minWidth = 78;
  int minHeight = 20;
  std::string stateFile = "state.dat";
  std::string keyFile = "key.dat";
  std::string dbFile = "markit.db";
  std::string version = std::to_string(MARKIT_MAJOR_VERSION) + "." +
                        std::to_string(MARKIT_MINOR_VERSION);
};
