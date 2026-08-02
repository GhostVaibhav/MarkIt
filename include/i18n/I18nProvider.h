#pragma once
#include <string>

class I18nProvider {
public:
  virtual ~I18nProvider() = default;
  virtual std::string get(const std::string& key) const = 0;
  virtual void loadLanguage(const std::string& langCode) = 0;
};
