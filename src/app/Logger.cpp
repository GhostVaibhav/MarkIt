#include "Logger.h"

#include <spdlog/sinks/basic_file_sink.h>

#include <iostream>
#include <utils/PathUtils.h>

Logger& Logger::getInstance() {
  static Logger instance;
  return instance;
}

Logger::Logger() {
  try {
    logger = spdlog::basic_logger_mt("markit", PathUtils::getExecutablePath() + "/logs/markit.log");
    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::seconds(3));
  } catch (const spdlog::spdlog_ex& ex) {
    std::cerr << "Log init failed: " << ex.what() << std::endl;
  }
}

std::shared_ptr<spdlog::logger> Logger::getLogger() { return logger; }
