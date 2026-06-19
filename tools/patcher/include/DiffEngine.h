#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include "json.hpp"

namespace patcher {

struct PatchOperation {
    // ... (unchanged)
    std::string op; // "+", "-", "*+", "*-"
    std::string path;
    
    // For +
    size_t from_bytes = 0;
    size_t to_bytes = 0;
    
    // For *+
    size_t start_bytes = 0; // Where to insert in original file
    
    // For *-
    size_t end_bytes = 0; // End of removal range

    nlohmann::json toCompactJson() const {
        nlohmann::json j = nlohmann::json::array();
        if (op == "+") {
            j.push_back(1);
            j.push_back(from_bytes);
            j.push_back(to_bytes);
        } else if (op == "-") {
            j.push_back(2);
        } else if (op == "*+") {
            j.push_back(3);
            j.push_back(start_bytes);
            j.push_back(from_bytes);
            j.push_back(to_bytes);
        } else if (op == "*-") {
            j.push_back(4);
            j.push_back(start_bytes);
            j.push_back(end_bytes);
        }
        return j;
    }
};

class DiffEngine {
public:
    DiffEngine(const std::string& oldDir, const std::string& newDir, const std::string& outDir);
    
    bool generatePatches();
    
    static std::atomic<size_t> currentMemoryUsage_;
    static size_t memoryLimitBytes_;

private:
    std::string oldDir_;
    std::string newDir_;
    std::string outDir_;
    
    std::vector<PatchOperation> operations_;
    std::vector<uint8_t> patchesBlob_; // The raw data to be written to patches.bin

    std::unordered_map<uint64_t, std::vector<size_t>> patchBlobDedup_;
    std::mutex stateMutex_;

    std::vector<PatchOperation> processFile(const std::filesystem::path& oldPath, const std::filesystem::path& newPath, const std::string& relPath);
    std::vector<PatchOperation> handleNewFile(const std::filesystem::path& newPath, const std::string& relPath);
    std::vector<PatchOperation> handleDeletedFile(const std::string& relPath);
    
    void emitStarPlusChunks(std::vector<PatchOperation>& localOperations,
                           const std::string& relPath,
                           size_t& cursor,
                           size_t& addedBytes,
                           const std::vector<uint8_t>& insertBytes);
    
    // Appends data to patchesBlob_ and returns the start offset
    size_t appendToBlob(const std::vector<uint8_t>& data);
    size_t appendToBlobWithDedup(const uint8_t* data, size_t len);
};

} // namespace patcher
