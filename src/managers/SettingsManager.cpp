#include "managers/SettingsManager.h"
#include "json.hpp"
#include <spdlog/spdlog.h>

SettingsManager::SettingsManager(const std::string& filepath)
    : FileManager(filepath), language("en"), loaded(false) {}

void SettingsManager::load() {
  if (loaded) return;
  auto content = readFile();
  if (content && !content->empty()) {
    try {
      nlohmann::json j = nlohmann::json::parse(*content);
      if (j.contains("language") && j["language"].is_string()) {
        language = j["language"].get<std::string>();
      }
    } catch (const std::exception& e) {
      spdlog::error("SettingsManager: Failed to parse settings file: {}", e.what());
    }
  }
  loaded = true;
  save(); // Ensure default gets saved if file was missing/corrupt
}

void SettingsManager::save() {
  nlohmann::json j;
  j["language"] = language;
  std::string dump = j.dump(2);
  writeFile(dump);
}

std::string SettingsManager::getLanguage() {
  load();
  return language;
}

void SettingsManager::setLanguage(const std::string& lang) {
  load();
  language = lang;
  save();
}
