#pragma once

#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <limits.h>
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
}
