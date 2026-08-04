#include "i18n/JsonI18nProvider.h"
#include "json.hpp"
#include "files/FileManager.h"
#include "utils/PathUtils.h"
#include "Logger.h"
#include "toggles.h"

#ifndef DEBUG
#include "locales_data.h"
#endif

JsonI18nProvider::JsonI18nProvider() {
  loadLanguage("en");
}

std::optional<std::string> JsonI18nProvider::get(const std::string& key) const {
  auto it = strings.find(key);
  if (it != strings.end()) {
    return it->second;
  }
  return std::nullopt; // Fallback to returning nullopt if not found
}

void JsonI18nProvider::loadLanguage(const std::string& langCode) {
#ifdef DEBUG
  std::string filePath = PathUtils::getExecutablePath() + "/locales/" + langCode + ".json";
  FileManager fm(filePath);
  auto content = fm.readFile();
  if (content) {
    try {
      nlohmann::json j = nlohmann::json::parse(*content);
      strings.clear();
      for (auto& el : j.items()) {
        if (el.value().is_string()) {
          strings[el.key()] = el.value().get<std::string>();
        }
      }
      spdlog::info("I18n: Loaded language pack '{}'", langCode);
    } catch (const std::exception& e) {
      spdlog::error("I18n: Failed to parse language pack '{}': {}", langCode, e.what());
    }
  } else {
    spdlog::warn("I18n: Language pack '{}' not found at {}", langCode, filePath);
  }
#else
  std::string contentStr = ObfuscatedLocales::get(langCode);
  if (!contentStr.empty()) {
    try {
      nlohmann::json j = nlohmann::json::parse(contentStr);
      strings.clear();
      for (auto& el : j.items()) {
        if (el.value().is_string()) {
          strings[el.key()] = el.value().get<std::string>();
        }
      }
      spdlog::info("I18n: Loaded embedded language pack '{}'", langCode);
    } catch (const std::exception& e) {
      spdlog::error("I18n: Failed to parse embedded language pack '{}': {}", langCode, e.what());
    }
  } else {
    spdlog::warn("I18n: Embedded language pack '{}' not found", langCode);
  }
#endif
}

std::vector<std::pair<std::string, std::string>> JsonI18nProvider::getAvailableLanguages() const {
  std::vector<std::pair<std::string, std::string>> langs;
#ifdef DEBUG
  std::string filePath = PathUtils::getExecutablePath() + "/locales/languages.json";
  FileManager fm(filePath);
  auto content = fm.readFile();
#else
  std::string contentStr = ObfuscatedLocales::get("languages");
  std::optional<std::string> content = contentStr.empty() ? std::nullopt : std::make_optional(contentStr);
#endif
  if (content) {
    try {
      nlohmann::json j = nlohmann::json::parse(*content);
      for (auto& el : j.items()) {
        if (el.value().is_string()) {
          langs.push_back({el.key(), el.value().get<std::string>()});
        }
      }
    } catch (const std::exception& e) {
      spdlog::error("I18n: Failed to parse languages manifest: {}", e.what());
    }
  }
  
  if (langs.empty()) {
    // Fallback if manifest is missing
    langs.push_back({"en", "English"});
  }
  return langs;
}
