#pragma once
#include <string>

namespace StringUtils {
  inline std::string truncateString(const std::string& str, int width) {
    if (width <= 0) return ""; 
    if ((int)str.length() > width && width > 3) {
      return str.substr(0, width - 3) + "...";
    }
    return (int)str.length() > width ? str.substr(0, width) : str;
  }
}
