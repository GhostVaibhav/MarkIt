#pragma once
#include "I18nProvider.h"
#include <unordered_map>
#include <string>
#include <optional>

class JsonI18nProvider : public I18nProvider {
public:
  JsonI18nProvider();
  ~JsonI18nProvider() override = default;

  std::optional<std::string> get(const std::string& key) const override;
  void loadLanguage(const std::string& langCode) override;
  std::vector<std::pair<std::string, std::string>> getAvailableLanguages() const override;

private:
  std::unordered_map<std::string, std::string> strings;
};
