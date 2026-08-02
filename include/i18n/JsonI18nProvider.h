#pragma once
#include "I18nProvider.h"
#include <unordered_map>
#include <string>

class JsonI18nProvider : public I18nProvider {
public:
  JsonI18nProvider();
  ~JsonI18nProvider() override = default;

  std::string get(const std::string& key) const override;
  void loadLanguage(const std::string& langCode) override;

private:
  std::unordered_map<std::string, std::string> strings;
};
