#pragma once
#include <spdlog/spdlog.h>

#include <memory>

class Logger {
 public:
  static Logger& getInstance();
  std::shared_ptr<spdlog::logger> getLogger();

 private:
  Logger();
  std::shared_ptr<spdlog::logger> logger;
};
