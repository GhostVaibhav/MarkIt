#pragma once
#include "files/FileManager.h"
#include <string>

class SettingsManager : public FileManager {
public:
  explicit SettingsManager(const std::string& filepath);

  std::string getLanguage();
  void setLanguage(const std::string& lang);

private:
  std::string language;
  bool loaded;

  void load();
  void save();
};
