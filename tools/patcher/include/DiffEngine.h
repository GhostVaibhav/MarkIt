#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include "json.hpp"

namespace patcher {

struct PatchOperation {
    std::string op; // "+", "-", "*+", "*-"
    std::string path;
    
    // For +
    size_t from_bytes = 0;
    size_t to_bytes = 0;
    
    // For *+
    size_t start_bytes = 0; // Where to insert in original file
    
    // For *-
    size_t end_bytes = 0; // End of removal range

    nlohmann::json toJson() const {
        nlohmann::json j;
        j["op"] = op;
        j["path"] = path;
        if (op == "+") {
            j["from_bytes"] = from_bytes;
            j["to_bytes"] = to_bytes;
        } else if (op == "*+") {
            j["start_bytes"] = start_bytes;
            j["from_bytes"] = from_bytes;
            j["to_bytes"] = to_bytes;
        } else if (op == "*-") {
            j["start_bytes"] = start_bytes;
            j["end_bytes"] = end_bytes;
        }
        return j;
    }
};

class DiffEngine {
public:
    DiffEngine(const std::string& oldDir, const std::string& newDir, const std::string& outDir);
    
    bool generatePatches();

private:
    std::string oldDir_;
    std::string newDir_;
    std::string outDir_;
    
    std::vector<PatchOperation> operations_;
    std::vector<uint8_t> patchesBlob_; // The raw data to be written to patches.bin

    void processFile(const std::filesystem::path& oldPath, const std::filesystem::path& newPath, const std::string& relPath);
    void handleNewFile(const std::filesystem::path& newPath, const std::string& relPath);
    void handleDeletedFile(const std::string& relPath);
    
    // Appends data to patchesBlob_ and returns the start offset
    size_t appendToBlob(const std::vector<uint8_t>& data);
};

} // namespace patcher
