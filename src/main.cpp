#include "Application.h"
#include "config/AppConfig.h"
#include "rang/rang.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void checkUpdater() {
    AppConfig config;
    fs::path updaterPath = config.updaterPath;
    fs::path updaterNewPath = updaterPath.string() + ".new";

    // 1. Swap .new if it exists
    if (fs::exists(updaterNewPath)) {
        try {
            if (fs::exists(updaterPath)) {
                fs::remove(updaterPath);
            }
            fs::rename(updaterNewPath, updaterPath);
        } catch (const std::exception& e) {
            std::cerr << rang::fg::red << "Failed to swap updater binary: " << e.what() << rang::style::reset << "\n";
        }
    }

    // 2. Verify updater exists
    if (!fs::exists(updaterPath)) {
        std::cerr << rang::fg::red << "\nFATAL ERROR: Updater executable not found at:\n"
                  << updaterPath.string() << "\n\n"
                  << "The MarkIt application must be packaged with its updater.\n"
                  << "Please reinstall the application." << rang::style::reset << "\n";
        exit(1);
    }

    // 3. Verify updater version
    std::string cmd;
#ifdef _WIN32
    cmd = "\"\"" + updaterPath.string() + "\" --version\"";
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    cmd = "\"" + updaterPath.string() + "\" --version";
    FILE* pipe = popen(cmd.c_str(), "r");
#endif

    if (!pipe) {
        std::cerr << rang::fg::red << "\nFATAL ERROR: Failed to execute updater to verify version.\n" << rang::style::reset << "\n";
        exit(1);
    }

    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    // Trim whitespace/newlines
    size_t last = result.find_last_not_of(" \n\r\t");
    if (last != std::string::npos) {
        result.erase(last + 1);
    } else {
        result.clear();
    }

    if (result != config.version) {
        std::cerr << rang::fg::red << "FATAL ERROR: Updater version mismatch.\n"
                  << "App Version:     " << config.version << "\n"
                  << "Updater Version: " << result << "\n\n"
                  << "The MarkIt application must be packaged with its identically versioned updater.\n"
                  << "Please reinstall the application." << rang::style::reset << "\n";
        exit(1);
    }
}

int main() {
  checkUpdater();
  Application app;
  return app.run();
}
