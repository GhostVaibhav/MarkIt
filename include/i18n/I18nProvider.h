#pragma once
#include <string>
#include <optional>
#include <vector>
#include <utility>

class I18nProvider {
public:
  virtual ~I18nProvider() = default;
  virtual std::optional<std::string> get(const std::string& key) const = 0;
  virtual void loadLanguage(const std::string& langCode) = 0;
  virtual std::vector<std::pair<std::string, std::string>> getAvailableLanguages() const = 0;
};
