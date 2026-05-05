#include "DiffEngine.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <functional>
#include <unordered_map>
#include "picosha2.h"

namespace fs = std::filesystem;

namespace patcher {

namespace {

enum class EditType {
    Equal,
    Delete,
    Insert
};

struct TraceRow {
    int minK = 0;
    std::vector<int> xByK;
};

int xAt(const TraceRow& row, int k) {
    const int idx = k - row.minK;
    if (idx < 0 || idx >= static_cast<int>(row.xByK.size())) {
        return std::numeric_limits<int>::min() / 4;
    }
    return row.xByK[static_cast<size_t>(idx)];
}

std::vector<EditType> computeMyersEditScript(
    int n,
    int m,
    const std::function<bool(int, int)>& isEqual
) {
    const int maxD = n + m;
    constexpr size_t kMaxTraceCells = 25'000'000; // ~100MB for int storage
    size_t traceCellsUsed = 0;

    TraceRow previousRow;
    previousRow.minK = 0;
    previousRow.xByK = {0};
    std::vector<TraceRow> trace;
    trace.reserve(1024);

    bool reachedEnd = false;
    int finalD = 0;

    for (int d = 0; d <= maxD && !reachedEnd; ++d) {
        TraceRow currentRow;
        currentRow.minK = -d;
        currentRow.xByK.assign(static_cast<size_t>(2 * d + 1), 0);

        traceCellsUsed += currentRow.xByK.size();
        if (traceCellsUsed > kMaxTraceCells) {
            throw std::runtime_error("Myers trace exceeded memory budget.");
        }

        for (int k = -d; k <= d; k += 2) {
            int x = 0;
            if (d == 0 && k == 0) {
                x = 0;
            } else if (k == -d || (k != d && xAt(previousRow, k - 1) < xAt(previousRow, k + 1))) {
                x = xAt(previousRow, k + 1);
            } else {
                x = xAt(previousRow, k - 1) + 1;
            }

            int y = x - k;
            while (x < n && y < m && isEqual(x, y)) {
                ++x;
                ++y;
            }

            currentRow.xByK[static_cast<size_t>(k - currentRow.minK)] = x;
            if (x >= n && y >= m) {
                reachedEnd = true;
                finalD = d;
                break;
            }
        }

        trace.push_back(std::move(currentRow));
        previousRow = trace.back();
    }

    if (!reachedEnd) {
        throw std::runtime_error("Myers diff failed to converge.");
    }

    std::vector<EditType> editsReversed;
    editsReversed.reserve(static_cast<size_t>(n + m));

    int x = n;
    int y = m;

    for (int d = finalD; d > 0; --d) {
        const auto& previousV = trace[static_cast<size_t>(d - 1)];
        const int k = x - y;

        int prevK = 0;
        if (k == -d || (k != d && xAt(previousV, k - 1) < xAt(previousV, k + 1))) {
            prevK = k + 1;
        } else {
            prevK = k - 1;
        }

        const int prevX = xAt(previousV, prevK);
        const int prevY = prevX - prevK;

        while (x > prevX && y > prevY) {
            editsReversed.push_back(EditType::Equal);
            --x;
            --y;
        }

        if (x == prevX) {
            editsReversed.push_back(EditType::Insert);
            --y;
        } else {
            editsReversed.push_back(EditType::Delete);
            --x;
        }
    }

    while (x > 0 && y > 0) {
        editsReversed.push_back(EditType::Equal);
        --x;
        --y;
    }
    while (x > 0) {
        editsReversed.push_back(EditType::Delete);
        --x;
    }
    while (y > 0) {
        editsReversed.push_back(EditType::Insert);
        --y;
    }

    std::reverse(editsReversed.begin(), editsReversed.end());
    return editsReversed;
}

std::vector<EditType> computeMyersEditScriptBytes(const std::vector<uint8_t>& oldData, const std::vector<uint8_t>& newData) {
    return computeMyersEditScript(
        static_cast<int>(oldData.size()),
        static_cast<int>(newData.size()),
        [&](int oldIdx, int newIdx) {
            return oldData[static_cast<size_t>(oldIdx)] == newData[static_cast<size_t>(newIdx)];
        }
    );
}

static uint64_t fnv1a64(const uint8_t* data, size_t len) {
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < len; ++i) {
        h ^= static_cast<uint64_t>(data[i]);
        h *= 1099511628211ULL;
    }
    return h;
}

std::vector<EditType> computeMyersEditScriptBlocks(
    const std::vector<uint8_t>& oldData,
    const std::vector<uint8_t>& newData,
    size_t blockSize
) {
    const size_t oldBlocks = (oldData.size() + blockSize - 1) / blockSize;
    const size_t newBlocks = (newData.size() + blockSize - 1) / blockSize;

    std::vector<uint64_t> oldHashes(oldBlocks);
    std::vector<uint64_t> newHashes(newBlocks);

    for (size_t i = 0; i < oldBlocks; ++i) {
        const size_t start = i * blockSize;
        const size_t len = std::min(blockSize, oldData.size() - start);
        oldHashes[i] = fnv1a64(oldData.data() + start, len) ^ static_cast<uint64_t>(len);
    }
    for (size_t i = 0; i < newBlocks; ++i) {
        const size_t start = i * blockSize;
        const size_t len = std::min(blockSize, newData.size() - start);
        newHashes[i] = fnv1a64(newData.data() + start, len) ^ static_cast<uint64_t>(len);
    }

    return computeMyersEditScript(
        static_cast<int>(oldBlocks),
        static_cast<int>(newBlocks),
        [&](int oldIdx, int newIdx) {
            return oldHashes[static_cast<size_t>(oldIdx)] == newHashes[static_cast<size_t>(newIdx)];
        }
    );
}

// Content-addressed reuse inside patches.bin: identical byte spans share one physical copy.
// Multiple *+ (and +) ops may reference the same [from_bytes, to_bytes) range.
static std::unordered_map<uint64_t, std::vector<size_t>> g_patchBlobDedup;

static void clearPatchBlobDedup() { g_patchBlobDedup.clear(); }

static size_t appendToBlobWithDedup(std::vector<uint8_t>& blob, const uint8_t* data, size_t len) {
    if (len == 0) {
        return blob.size();
    }
    const uint64_t h = fnv1a64(data, len) ^ static_cast<uint64_t>(len);
    auto& candidates = g_patchBlobDedup[h];
    for (size_t off : candidates) {
        if (off + len <= blob.size() &&
            std::equal(data, data + len, blob.begin() + static_cast<std::ptrdiff_t>(off))) {
            return off;
        }
    }
    const size_t start = blob.size();
    blob.insert(blob.end(), data, data + len);
    candidates.push_back(start);
    return start;
}

// Chunk large *+ payloads so recurring chunks can dedupe; slightly larger instructions.json.
static constexpr size_t kStarPlusChunkBytes = 256 * 1024;

static size_t matchingSuffixPrefix(const std::vector<uint8_t>& blob, const uint8_t* data, size_t len) {
    const size_t maxN = std::min(len, blob.size());
    size_t n = 0;
    while (n < maxN && blob[blob.size() - 1 - n] == data[n]) {
        ++n;
    }
    return n;
}

static void emitStarPlusChunks(std::vector<PatchOperation>& operations,
                               std::vector<uint8_t>& patchesBlob,
                               const std::string& relPath,
                               size_t& cursor,
                               size_t& addedBytes,
                               const std::vector<uint8_t>& insertBytes) {
    for (size_t off = 0; off < insertBytes.size(); off += kStarPlusChunkBytes) {
        const size_t len = std::min(kStarPlusChunkBytes, insertBytes.size() - off);
        const uint8_t* chunkPtr = insertBytes.data() + off;

        const size_t overlap = matchingSuffixPrefix(patchesBlob, chunkPtr, len);
        if (overlap > 0) {
            const size_t from = patchesBlob.size() - overlap;
            PatchOperation reuseOp;
            reuseOp.op = "*+";
            reuseOp.path = relPath;
            reuseOp.start_bytes = cursor;
            reuseOp.from_bytes = from;
            reuseOp.to_bytes = from + overlap;
            operations.push_back(reuseOp);
            cursor += overlap;
            addedBytes += overlap;
        }

        const size_t tail = len - overlap;
        if (tail == 0) {
            continue;
        }

        const size_t blobStart = appendToBlobWithDedup(patchesBlob, chunkPtr + overlap, tail);

        PatchOperation addOp;
        addOp.op = "*+";
        addOp.path = relPath;
        addOp.start_bytes = cursor;
        addOp.from_bytes = blobStart;
        addOp.to_bytes = blobStart + tail;
        operations.push_back(addOp);

        cursor += tail;
        addedBytes += tail;
    }
}

} // namespace

DiffEngine::DiffEngine(const std::string& oldDir, const std::string& newDir, const std::string& outDir)
    : oldDir_(oldDir), newDir_(newDir), outDir_(outDir) {}

size_t DiffEngine::appendToBlob(const std::vector<uint8_t>& data) {
    return appendToBlobWithDedup(patchesBlob_, data.data(), data.size());
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
    clearPatchBlobDedup();

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
    
    std::vector<EditType> edits;
    bool usedBlockMyers = false;
    size_t selectedBlockSize = 0;
    try {
        constexpr size_t kLargeFileThresholdBytes = 4 * 1024 * 1024;
        constexpr size_t kInitialBlockSize = 1024;
        const size_t maxFileSize = std::max(oldData.size(), newData.size());
        const bool preferByteMyers = maxFileSize <= kLargeFileThresholdBytes;

        if (preferByteMyers) {
            edits = computeMyersEditScriptBytes(oldData, newData);
            usedBlockMyers = false;
        } else {
            size_t blockSize = kInitialBlockSize;
            bool solved = false;
            while (!solved) {
                try {
                    edits = computeMyersEditScriptBlocks(oldData, newData, blockSize);
                    usedBlockMyers = true;
                    selectedBlockSize = blockSize;
                    solved = true;
                } catch (const std::runtime_error&) {
                    if (blockSize >= maxFileSize && maxFileSize > 0) {
                        break;
                    }
                    if (maxFileSize == 0) {
                        break;
                    }
                    blockSize = std::min(maxFileSize, blockSize * 2);
                    if (blockSize == 0) {
                        blockSize = maxFileSize;
                    }
                }
            }

            if (!solved) {
                throw std::runtime_error("Myers block diff failed after retries.");
            }
        }
    } catch (const std::exception&) {
        // If byte-level Myers still exceeds budget on a small-but-high-diff file,
        // retry with progressively larger block sizes to guarantee completion.
        const size_t maxFileSize = std::max(oldData.size(), newData.size());
        size_t blockSize = 1024;
        bool solved = false;
        while (!solved) {
            try {
                edits = computeMyersEditScriptBlocks(oldData, newData, blockSize);
                usedBlockMyers = true;
                selectedBlockSize = blockSize;
                solved = true;
            } catch (const std::runtime_error&) {
                if (blockSize >= maxFileSize && maxFileSize > 0) {
                    break;
                }
                if (maxFileSize == 0) {
                    break;
                }
                blockSize = std::min(maxFileSize, blockSize * 2);
                if (blockSize == 0) {
                    blockSize = maxFileSize;
                }
            }
        }

        if (!solved) {
            throw std::runtime_error("Failed diff for '" + relPath + "': Myers retries exhausted.");
        }
    }

    size_t removedBytes = 0;
    size_t addedBytes = 0;
    size_t cursor = 0; // Cursor over the evolving file state during patch application
    size_t newIndex = 0;
    size_t oldIndex = 0;
    size_t refinedSegments = 0;

    auto emitByteLevelOpsForSegments =
        [&](const std::vector<uint8_t>& oldSeg, const std::vector<uint8_t>& newSeg, size_t startCursor) -> size_t {
            const std::vector<EditType> segEdits = computeMyersEditScriptBytes(oldSeg, newSeg);
            size_t localCursor = startCursor;
            size_t segNewIndex = 0;
            size_t segOldIndex = 0;

            for (size_t si = 0; si < segEdits.size();) {
                if (segEdits[si] == EditType::Equal) {
                    ++localCursor;
                    ++segOldIndex;
                    ++segNewIndex;
                    ++si;
                    continue;
                }

                if (segEdits[si] == EditType::Delete) {
                    const size_t start = localCursor;
                    size_t runLen = 0;
                    while (si < segEdits.size() && segEdits[si] == EditType::Delete) {
                        ++runLen;
                        ++segOldIndex;
                        ++si;
                    }

                    PatchOperation removeOp;
                    removeOp.op = "*-";
                    removeOp.path = relPath;
                    removeOp.start_bytes = start;
                    removeOp.end_bytes = start + runLen;
                    operations_.push_back(removeOp);
                    removedBytes += runLen;
                    continue;
                }

                std::vector<uint8_t> insertBytes;
                while (si < segEdits.size() && segEdits[si] == EditType::Insert) {
                    insertBytes.push_back(newSeg[segNewIndex]);
                    ++segNewIndex;
                    ++si;
                }

                if (!insertBytes.empty()) {
                    emitStarPlusChunks(operations_, patchesBlob_, relPath, localCursor, addedBytes, insertBytes);
                }
            }

            return localCursor;
        };

    for (size_t i = 0; i < edits.size();) {
        if (edits[i] == EditType::Equal) {
            if (usedBlockMyers) {
                const size_t oldRemain = oldData.size() - oldIndex;
                const size_t newRemain = newData.size() - newIndex;
                const size_t equalLen = std::min({selectedBlockSize, oldRemain, newRemain});
                cursor += equalLen;
                oldIndex += equalLen;
                newIndex += equalLen;
            } else {
                ++cursor;
                ++oldIndex;
                ++newIndex;
            }
            ++i;
            continue;
        }

        if (edits[i] == EditType::Delete) {
            const size_t start = cursor;
            const size_t deleteOldStart = oldIndex;
            size_t runLen = 0;
            while (i < edits.size() && edits[i] == EditType::Delete) {
                if (usedBlockMyers) {
                    const size_t oldRemain = oldData.size() - oldIndex;
                    const size_t len = std::min(selectedBlockSize, oldRemain);
                    runLen += len;
                    oldIndex += len;
                } else {
                    ++runLen;
                    ++oldIndex;
                }
                ++i;
            }

            if (usedBlockMyers && i < edits.size() && edits[i] == EditType::Insert) {
                const size_t insertNewStart = newIndex;
                size_t insertRunLen = 0;
                size_t lookahead = i;
                while (lookahead < edits.size() && edits[lookahead] == EditType::Insert) {
                    const size_t newRemain = newData.size() - (insertNewStart + insertRunLen);
                    const size_t len = std::min(selectedBlockSize, newRemain);
                    insertRunLen += len;
                    ++lookahead;
                }

                constexpr size_t kRefineMaxSegmentBytes = 8 * 1024 * 1024;
                if (runLen > 0 && insertRunLen > 0 &&
                    runLen <= kRefineMaxSegmentBytes &&
                    insertRunLen <= kRefineMaxSegmentBytes) {
                    try {
                        std::vector<uint8_t> oldSeg(
                            oldData.begin() + static_cast<std::ptrdiff_t>(deleteOldStart),
                            oldData.begin() + static_cast<std::ptrdiff_t>(deleteOldStart + runLen));
                        std::vector<uint8_t> newSeg(
                            newData.begin() + static_cast<std::ptrdiff_t>(insertNewStart),
                            newData.begin() + static_cast<std::ptrdiff_t>(insertNewStart + insertRunLen));

                        cursor = emitByteLevelOpsForSegments(oldSeg, newSeg, start);
                        newIndex += insertRunLen;
                        i = lookahead;
                        ++refinedSegments;
                        continue;
                    } catch (const std::exception&) {
                        // Keep coarse block-level replace if refinement exceeds budget.
                    }
                }
            }

            PatchOperation removeOp;
            removeOp.op = "*-";
            removeOp.path = relPath;
            removeOp.start_bytes = start;
            removeOp.end_bytes = start + runLen;
            operations_.push_back(removeOp);
            removedBytes += runLen;
            continue;
        }

        std::vector<uint8_t> insertBytes;
        while (i < edits.size() && edits[i] == EditType::Insert) {
            if (usedBlockMyers) {
                const size_t newRemain = newData.size() - newIndex;
                const size_t len = std::min(selectedBlockSize, newRemain);
                insertBytes.insert(insertBytes.end(), newData.begin() + static_cast<std::ptrdiff_t>(newIndex),
                                   newData.begin() + static_cast<std::ptrdiff_t>(newIndex + len));
                newIndex += len;
            } else {
                insertBytes.push_back(newData[newIndex]);
                ++newIndex;
            }
            ++i;
        }

        if (!insertBytes.empty()) {
            emitStarPlusChunks(operations_, patchesBlob_, relPath, cursor, addedBytes, insertBytes);
        }
    }

    std::cout << "  [MOD] " << relPath << " (-" << removedBytes << " bytes, +" << addedBytes << " bytes)"
              << (usedBlockMyers ? " [myers-block:" + std::to_string(selectedBlockSize) + "]" : " [myers-byte]")
              << (refinedSegments > 0 ? " [refined:" + std::to_string(refinedSegments) + "]" : "")
              << "\n";
}

} // namespace patcher
