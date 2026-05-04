#pragma once
#include <string>

#include "toggles.h"
#include "utils/PathUtils.h"
#include "config/UpdateConfig.h"

struct AppConfig {
  const int minWidth = 78;
  const int minHeight = 20;
  const std::string stateFile = (std::filesystem::path(PathUtils::getDataPath()) / "state.dat").string();
  const std::string keyFile = (std::filesystem::path(PathUtils::getDataPath()) / "key.dat").string();
  const std::string dbFile = (std::filesystem::path(PathUtils::getDataPath()) / "markit.db").string();
  const std::string version = std::to_string(MARKIT_MAJOR_VERSION) + "." +
                        std::to_string(MARKIT_MINOR_VERSION) + "." +
                        std::to_string(MARKIT_PATCH_VERSION);
  const std::string updaterPath = (std::filesystem::path(PathUtils::getExecutablePath()) / UpdateConfig::kUpdaterBinary).string();
};
