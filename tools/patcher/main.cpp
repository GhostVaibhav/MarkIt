#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

#include "DiffEngine.h"

void printUsage(const char* argv0) {
    std::cout << "Usage: " << argv0 << " <old_dir> <new_dir> <output_dir>\n"
              << "Generates binary patches and instructions.json from old_dir to new_dir.\n";
}

int main(int argc, char** argv) {
    if (argc != 4) {
        printUsage(argv[0]);
        return 1;
    }

    std::string oldDir = argv[1];
    std::string newDir = argv[2];
    std::string outDir = argv[3];

    if (!fs::exists(oldDir) || !fs::is_directory(oldDir)) {
        std::cerr << "Error: old_dir does not exist or is not a directory: " << oldDir << "\n";
        return 1;
    }

    if (!fs::exists(newDir) || !fs::is_directory(newDir)) {
        std::cerr << "Error: new_dir does not exist or is not a directory: " << newDir << "\n";
        return 1;
    }

    if (!fs::exists(outDir)) {
        fs::create_directories(outDir);
    }

    std::cout << "Patcher started.\n"
              << "Old Dir: " << oldDir << "\n"
              << "New Dir: " << newDir << "\n"
              << "Out Dir: " << outDir << "\n";

    patcher::DiffEngine engine(oldDir, newDir, outDir);
    if (!engine.generatePatches()) {
        std::cerr << "Failed to generate patches.\n";
        return 1;
    }
    
    std::cout << "Patcher completed successfully. Output written to " << outDir << "\n";
    return 0;
}
