#pragma once

#include <filesystem>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <climits.h>
#include <cstdlib>
#include <unistd.h>
#endif

namespace PathUtils {
	inline std::string getExecutablePath() {
	#ifdef _WIN32
		wchar_t path[8192] = { 0 };
		GetModuleFileNameW(NULL, path, 8192);
		return std::filesystem::path(path).parent_path().u8string();
	#else
		char result[8192];
		ssize_t count = readlink("/proc/self/exe", result, 8192);
		return std::filesystem::path(std::string(result, (count > 0) ? count : 0)).parent_path().u8string();
	#endif
	}

	// Returns a user-writable directory for storing data files (db, state, key, logs).
	// On Linux, this is ~/.markit/ (since the binary may be in /usr/local/bin).
	// On Windows, data files live next to the executable.
	inline std::string getDataPath() {
	#ifdef _WIN32
		return getExecutablePath();
	#else
		const char* home = std::getenv("HOME");
		std::string dataDir = std::string(home ? home : "/tmp") + "/.markit";
		std::filesystem::create_directories(dataDir);
		return dataDir;
	#endif
	}
}
