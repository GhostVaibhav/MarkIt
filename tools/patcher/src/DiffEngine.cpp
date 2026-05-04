#include "DiffEngine.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include "picosha2.h"

namespace fs = std::filesystem;

namespace patcher {

DiffEngine::DiffEngine(const std::string& oldDir, const std::string& newDir, const std::string& outDir)
    : oldDir_(oldDir), newDir_(newDir), outDir_(outDir) {}

size_t DiffEngine::appendToBlob(const std::vector<uint8_t>& data) {
    size_t start = patchesBlob_.size();
    patchesBlob_.insert(patchesBlob_.end(), data.begin(), data.end());
    return start;
}

static std::vector<uint8_t> readFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

static std::string hashFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return "";
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), hash.begin(), hash.end());
    return picosha2::bytes_to_hex_string(hash.begin(), hash.end());
}

bool DiffEngine::generatePatches() {
    nlohmann::json instructions;
    instructions["files"] = nlohmann::json::array();

    // Iterate through new directory to find new/modified files
    for (const auto& entry : fs::recursive_directory_iterator(newDir_)) {
        if (!entry.is_regular_file()) continue;
        
        fs::path relPath = fs::relative(entry.path(), newDir_);
        fs::path oldPath = fs::path(oldDir_) / relPath;
        
        nlohmann::json fileJson;
        fileJson["path"] = relPath.generic_string(); // Force forward slashes
        fileJson["hash"] = hashFile(entry.path());
        fileJson["operations"] = nlohmann::json::array();

        size_t startOpCount = operations_.size();

        if (!fs::exists(oldPath)) {
            handleNewFile(entry.path(), relPath.generic_string());
        } else {
            processFile(oldPath, entry.path(), relPath.generic_string());
        }

        if (operations_.size() > startOpCount) {
            for (size_t i = startOpCount; i < operations_.size(); ++i) {
                fileJson["operations"].push_back(operations_[i].toJson());
            }
            instructions["files"].push_back(fileJson);
        }
    }

    // Iterate through old directory to find deleted files
    for (const auto& entry : fs::recursive_directory_iterator(oldDir_)) {
        if (!entry.is_regular_file()) continue;
        
        fs::path relPath = fs::relative(entry.path(), oldDir_);
        fs::path newPath = fs::path(newDir_) / relPath;
        
        if (!fs::exists(newPath)) {
            nlohmann::json fileJson;
            fileJson["path"] = relPath.generic_string();
            fileJson["operations"] = nlohmann::json::array();
            
            handleDeletedFile(relPath.generic_string());
            fileJson["operations"].push_back(operations_.back().toJson());
            
            instructions["files"].push_back(fileJson);
        }
    }

    // Write out patches.bin
    fs::path binPath = fs::path(outDir_) / "patches.bin";
    std::ofstream binOut(binPath, std::ios::binary);
    if (!binOut) {
        std::cerr << "Failed to open " << binPath << " for writing.\n";
        return false;
    }
    binOut.write(reinterpret_cast<const char*>(patchesBlob_.data()), patchesBlob_.size());

    // Write out instructions.json
    fs::path jsonPath = fs::path(outDir_) / "instructions.json";
    std::ofstream jsonOut(jsonPath);
    if (!jsonOut) {
        std::cerr << "Failed to open " << jsonPath << " for writing.\n";
        return false;
    }
    jsonOut << instructions.dump(2);

    return true;
}

void DiffEngine::handleNewFile(const fs::path& newPath, const std::string& relPath) {
    auto data = readFile(newPath);
    size_t start = appendToBlob(data);
    
    std::cout << "  [NEW] " << relPath << " (" << data.size() << " bytes)\n";

    PatchOperation op;
    op.op = "+";
    op.path = relPath;
    op.from_bytes = start;
    op.to_bytes = start + data.size();
    operations_.push_back(op);
}

void DiffEngine::handleDeletedFile(const std::string& relPath) {
    std::cout << "  [DEL] " << relPath << "\n";
    
    PatchOperation op;
    op.op = "-";
    op.path = relPath;
    operations_.push_back(op);
}

void DiffEngine::processFile(const fs::path& oldPath, const fs::path& newPath, const std::string& relPath) {
    auto oldData = readFile(oldPath);
    auto newData = readFile(newPath);
    
    if (oldData == newData) {
        std::cout << "  [UNCHANGED] " << relPath << "\n";
        return; // Unchanged
    }
    
    // Find common prefix
    auto prefixPair = std::mismatch(oldData.begin(), oldData.end(), newData.begin(), newData.end());
    size_t prefixLen = std::distance(oldData.begin(), prefixPair.first);
    
    // Find common suffix
    auto oldRevBegin = oldData.rbegin();
    auto oldRevEnd = oldData.rend() - prefixLen; // Don't cross prefix
    auto newRevBegin = newData.rbegin();
    auto newRevEnd = newData.rend() - prefixLen;
    
    auto suffixPair = std::mismatch(oldRevBegin, oldRevEnd, newRevBegin, newRevEnd);
    size_t suffixLen = std::distance(oldRevBegin, suffixPair.first);
    
    size_t oldMiddleLen = oldData.size() - prefixLen - suffixLen;
    size_t newMiddleLen = newData.size() - prefixLen - suffixLen;
    
    // Optimization disabled for testing: Always output patch instructions regardless of change size

    std::cout << "  [MOD] " << relPath << " (-" << oldMiddleLen << " bytes, +" << newMiddleLen << " bytes)\n";

    // It's a modify operation!
    // Note: User constraint specifies we do *- BEFORE *+.
    
    if (oldMiddleLen > 0) {
        PatchOperation removeOp;
        removeOp.op = "*-";
        removeOp.path = relPath;
        removeOp.start_bytes = prefixLen;
        removeOp.end_bytes = prefixLen + oldMiddleLen;
        operations_.push_back(removeOp);
    }
    
    if (newMiddleLen > 0) {
        std::vector<uint8_t> newBytes(newData.begin() + prefixLen, newData.begin() + prefixLen + newMiddleLen);
        size_t start = appendToBlob(newBytes);
        
        PatchOperation addOp;
        addOp.op = "*+";
        addOp.path = relPath;
        addOp.start_bytes = prefixLen;
        addOp.from_bytes = start;
        addOp.to_bytes = start + newMiddleLen;
        operations_.push_back(addOp);
    }
}

} // namespace patcher
