#include "DiffEngine.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <functional>
#include <unordered_map>
#include <array>
#include <queue>
#include <future>
#include <thread>
#include "picosha2.h"

namespace fs = std::filesystem;

namespace patcher {

std::atomic<size_t> DiffEngine::currentMemoryUsage_{0};
size_t DiffEngine::memoryLimitBytes_{2048ULL * 1024 * 1024};

namespace {

enum class EditType {
    Equal,
    Delete,
    Insert
};

struct MemoryGuard {
    size_t bytes = 0;
    ~MemoryGuard() {
        DiffEngine::currentMemoryUsage_.fetch_sub(bytes);
    }
    void add(size_t b) {
        if (DiffEngine::currentMemoryUsage_.fetch_add(b) + b > DiffEngine::memoryLimitBytes_) {
            DiffEngine::currentMemoryUsage_.fetch_sub(b);
            throw std::runtime_error("Patcher exceeded global memory limit (2048 MB).");
        }
        bytes += b;
    }
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

    MemoryGuard mg;
    mg.add(previousRow.xByK.size() * sizeof(int));

    bool reachedEnd = false;
    int finalD = 0;

    for (int d = 0; d <= maxD && !reachedEnd; ++d) {
        TraceRow currentRow;
        currentRow.minK = -d;
        const size_t rowSize = static_cast<size_t>(2 * d + 1);
        currentRow.xByK.assign(rowSize, 0);

        mg.add(rowSize * sizeof(int));
        traceCellsUsed += rowSize;
        if (traceCellsUsed > kMaxTraceCells) {
             // We don't need this local check anymore if we have a global one, 
             // but let's keep it for safety as a per-file cap.
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

// Chunk large *+ payloads so recurring chunks can dedupe; slightly larger instructions.json.
static constexpr size_t kStarPlusChunkBytes = 256 * 1024;

static size_t matchingSuffixPrefix(const std::vector<uint8_t>& blob, const uint8_t* data, size_t len) {
    const size_t maxN = std::min({len, blob.size(), size_t(4096)}); // Cap for performance
    for (size_t n = maxN; n > 0; --n) {
        bool match = true;
        for (size_t i = 0; i < n; ++i) {
            if (blob[blob.size() - n + i] != data[i]) {
                match = false;
                break;
            }
        }
        if (match) return n;
    }
    return 0;
}


static void writeU64LE(std::vector<uint8_t>& out, uint64_t v) {
    for (int i = 0; i < 8; ++i) {
        out.push_back(static_cast<uint8_t>((v >> (i * 8)) & 0xFFu));
    }
}

struct HuffNode {
    uint64_t freq = 0;
    int left = -1;
    int right = -1;
    int symbol = -1;
};

static bool buildHuffmanCodeLengths(const std::vector<uint8_t>& data, std::array<uint8_t, 256>& lengths) {
    lengths.fill(0);
    if (data.empty()) return false;

    std::array<uint64_t, 256> freq{};
    for (uint8_t b : data) ++freq[b];

    std::vector<HuffNode> nodes;
    nodes.reserve(512);

    using PQItem = std::pair<uint64_t, int>;
    auto cmp = [](const PQItem& a, const PQItem& b) {
        if (a.first != b.first) return a.first > b.first;
        return a.second > b.second;
    };
    std::priority_queue<PQItem, std::vector<PQItem>, decltype(cmp)> pq(cmp);

    for (int s = 0; s < 256; ++s) {
        if (freq[static_cast<size_t>(s)] == 0) continue;
        nodes.push_back(HuffNode{freq[static_cast<size_t>(s)], -1, -1, s});
        pq.push({freq[static_cast<size_t>(s)], static_cast<int>(nodes.size()) - 1});
    }

    if (pq.size() == 1) {
        lengths[static_cast<size_t>(nodes[0].symbol)] = 1;
        return true;
    }

    while (pq.size() > 1) {
        auto [f1, i1] = pq.top();
        pq.pop();
        auto [f2, i2] = pq.top();
        pq.pop();
        (void)f1;
        (void)f2;

        const int parent = static_cast<int>(nodes.size());
        nodes.push_back(HuffNode{nodes[static_cast<size_t>(i1)].freq + nodes[static_cast<size_t>(i2)].freq, i1, i2, -1});
        pq.push({nodes.back().freq, parent});
    }

    const int root = pq.top().second;
    std::vector<std::pair<int, int>> stack;
    stack.push_back({root, 0});
    while (!stack.empty()) {
        auto [idx, depth] = stack.back();
        stack.pop_back();
        const HuffNode& n = nodes[static_cast<size_t>(idx)];
        if (n.symbol >= 0) {
            lengths[static_cast<size_t>(n.symbol)] = static_cast<uint8_t>(std::max(1, depth));
            continue;
        }
        if (n.left >= 0) stack.push_back({n.left, depth + 1});
        if (n.right >= 0) stack.push_back({n.right, depth + 1});
    }

    return true;
}

static bool encodeHuffman(const std::vector<uint8_t>& data, std::vector<uint8_t>& encoded, std::array<uint8_t, 256>& lengths) {
    if (!buildHuffmanCodeLengths(data, lengths)) return false;

    struct SymbolLen { int sym; uint8_t len; };
    std::vector<SymbolLen> syms;
    syms.reserve(256);
    for (int s = 0; s < 256; ++s) {
        if (lengths[static_cast<size_t>(s)] > 0) syms.push_back({s, lengths[static_cast<size_t>(s)]});
    }
    std::sort(syms.begin(), syms.end(), [](const SymbolLen& a, const SymbolLen& b) {
        if (a.len != b.len) return a.len < b.len;
        return a.sym < b.sym;
    });

    std::array<uint32_t, 256> codes{};
    uint32_t code = 0;
    uint8_t prevLen = syms.empty() ? 0 : syms[0].len;
    for (const auto& s : syms) {
        if (s.len > prevLen) {
            code <<= (s.len - prevLen);
            prevLen = s.len;
        }
        codes[static_cast<size_t>(s.sym)] = code;
        ++code;
    }

    encoded.clear();
    encoded.reserve(data.size());
    uint8_t cur = 0;
    int bitsFilled = 0;
    for (uint8_t b : data) {
        const uint8_t len = lengths[b];
        uint32_t c = codes[b];
        for (int bit = static_cast<int>(len) - 1; bit >= 0; --bit) {
            const uint8_t v = static_cast<uint8_t>((c >> bit) & 1u);
            cur = static_cast<uint8_t>((cur << 1) | v);
            ++bitsFilled;
            if (bitsFilled == 8) {
                encoded.push_back(cur);
                cur = 0;
                bitsFilled = 0;
            }
        }
    }
    if (bitsFilled > 0) {
        cur = static_cast<uint8_t>(cur << (8 - bitsFilled));
        encoded.push_back(cur);
    }
    return true;
}

static std::vector<uint8_t> buildCompressedPatchBlobIfSmaller(const std::vector<uint8_t>& raw) {
    std::array<uint8_t, 256> lengths{};
    std::vector<uint8_t> encoded;
    if (!encodeHuffman(raw, encoded, lengths)) return {};

    MemoryGuard mg;
    mg.add(encoded.size() + 512); // encoded + metadata approx

    std::vector<uint8_t> container;
    container.reserve(4 + 1 + 3 + 8 + 8 + 256 + encoded.size());
    container.push_back('M');
    container.push_back('K');
    container.push_back('P');
    container.push_back('1');
    container.push_back(1); // method: 1 = Huffman
    container.push_back(0);
    container.push_back(0);
    container.push_back(0);
    writeU64LE(container, static_cast<uint64_t>(raw.size()));
    writeU64LE(container, static_cast<uint64_t>(encoded.size()));
    container.insert(container.end(), lengths.begin(), lengths.end());
    container.insert(container.end(), encoded.begin(), encoded.end());

    if (container.size() >= raw.size()) return {};
    return container;
}

static std::vector<uint8_t> encodeRle(const std::vector<uint8_t>& raw) {
    std::vector<uint8_t> out;
    out.reserve(raw.size());
    size_t i = 0;
    while (i < raw.size()) {
        size_t run = 1;
        while (i + run < raw.size() && raw[i + run] == raw[i] && run < 130) {
            ++run;
        }
        if (run >= 3) {
            out.push_back(static_cast<uint8_t>(0x80u | static_cast<uint8_t>(run - 3)));
            out.push_back(raw[i]);
            i += run;
            continue;
        }

        const size_t litStart = i;
        size_t litLen = 0;
        while (i < raw.size() && litLen < 128) {
            size_t aheadRun = 1;
            while (i + aheadRun < raw.size() && raw[i + aheadRun] == raw[i] && aheadRun < 130) {
                ++aheadRun;
            }
            if (aheadRun >= 3) break;
            ++i;
            ++litLen;
        }

        out.push_back(static_cast<uint8_t>(litLen - 1));
        out.insert(out.end(), raw.begin() + static_cast<std::ptrdiff_t>(litStart),
                   raw.begin() + static_cast<std::ptrdiff_t>(litStart + litLen));
    }
    return out;
}

static std::vector<uint8_t> buildRleCompressedPatchBlobIfSmaller(const std::vector<uint8_t>& raw) {
    if (raw.empty()) return {};
    std::vector<uint8_t> payload = encodeRle(raw);

    MemoryGuard mg;
    mg.add(payload.size() + 64);

    std::vector<uint8_t> container;
    container.reserve(4 + 1 + 3 + 8 + 8 + payload.size());
    container.push_back('M');
    container.push_back('K');
    container.push_back('P');
    container.push_back('1');
    container.push_back(2); // method: 2 = RLE
    container.push_back(0);
    container.push_back(0);
    container.push_back(0);
    writeU64LE(container, static_cast<uint64_t>(raw.size()));
    writeU64LE(container, static_cast<uint64_t>(payload.size()));
    container.insert(container.end(), payload.begin(), payload.end());

    if (container.size() >= raw.size()) return {};
    return container;
}

static void writeU32LE(std::vector<uint8_t>& out, uint32_t v) {
    for (int i = 0; i < 4; ++i) {
        out.push_back(static_cast<uint8_t>((v >> (i * 8)) & 0xFFu));
    }
}

static std::vector<uint8_t> encodeLz(const std::vector<uint8_t>& raw) {
    // Token stream:
    //  literal-run: [0..127] => len = tag+1, followed by len raw bytes
    //  backref-run: [128..255] => len = (tag-128)+3, followed by uint32 distance (LE)
    constexpr size_t kMinMatch = 3;
    constexpr size_t kMaxMatch = 130;
    constexpr size_t kMaxCandidatesPerKey = 4096;
    constexpr size_t kWindow = 32 * 1024 * 1024;

    std::vector<uint8_t> out;
    out.reserve(raw.size());

    std::unordered_map<uint32_t, std::vector<size_t>> positionsByKey;
    positionsByKey.reserve(raw.size() / 4 + 1);

    auto keyAt = [&](size_t i) -> uint32_t {
        return (static_cast<uint32_t>(raw[i]) << 16) |
               (static_cast<uint32_t>(raw[i + 1]) << 8) |
               static_cast<uint32_t>(raw[i + 2]);
    };

    auto appendLiteralRun = [&](size_t start, size_t len) {
        while (len > 0) {
            const size_t chunk = std::min<size_t>(128, len);
            out.push_back(static_cast<uint8_t>(chunk - 1));
            out.insert(out.end(),
                       raw.begin() + static_cast<std::ptrdiff_t>(start),
                       raw.begin() + static_cast<std::ptrdiff_t>(start + chunk));
            start += chunk;
            len -= chunk;
        }
    };

    size_t i = 0;
    size_t litStart = 0;
    size_t litLen = 0;

    auto flushLit = [&]() {
        if (litLen > 0) {
            appendLiteralRun(litStart, litLen);
            litLen = 0;
        }
    };

    while (i < raw.size()) {
        size_t bestLen = 0;
        size_t bestDist = 0;

        if (i + kMinMatch <= raw.size()) {
            const uint32_t key = keyAt(i);
            auto it = positionsByKey.find(key);
            if (it != positionsByKey.end()) {
                auto& candidates = it->second;
                for (auto rit = candidates.rbegin(); rit != candidates.rend(); ++rit) {
                    const size_t p = *rit;
                    if (p >= i) continue;
                    const size_t dist = i - p;
                    if (dist > kWindow || dist > static_cast<size_t>(std::numeric_limits<uint32_t>::max())) break;

                    size_t m = 0;
                    const size_t maxM = std::min(kMaxMatch, raw.size() - i);
                    while (m < maxM && raw[p + m] == raw[i + m]) {
                        ++m;
                    }
                    if (m >= kMinMatch && m > bestLen) {
                        bestLen = m;
                        bestDist = dist;
                        if (bestLen == kMaxMatch) break;
                    }
                }
            }
        }

        if (bestLen >= kMinMatch) {
            flushLit();
            out.push_back(static_cast<uint8_t>(0x80u | static_cast<uint8_t>(bestLen - kMinMatch)));
            writeU32LE(out, static_cast<uint32_t>(bestDist));

            const size_t end = i + bestLen;
            for (size_t j = i; j < end; ++j) {
                if (j + kMinMatch <= raw.size()) {
                    const uint32_t k = keyAt(j);
                    auto& vec = positionsByKey[k];
                    vec.push_back(j);
                    if (vec.size() > kMaxCandidatesPerKey) {
                        vec.erase(vec.begin(), vec.begin() + static_cast<std::ptrdiff_t>(vec.size() - kMaxCandidatesPerKey));
                    }
                }
            }
            i = end;
            litStart = i;
        } else {
            if (litLen == 0) litStart = i;
            ++litLen;
            if (i + kMinMatch <= raw.size()) {
                const uint32_t k = keyAt(i);
                auto& vec = positionsByKey[k];
                vec.push_back(i);
                if (vec.size() > kMaxCandidatesPerKey) {
                    vec.erase(vec.begin(), vec.begin() + static_cast<std::ptrdiff_t>(vec.size() - kMaxCandidatesPerKey));
                }
            }
            ++i;
        }
    }
    flushLit();
    return out;
}

static std::vector<uint8_t> buildLzCompressedPatchBlobIfSmaller(const std::vector<uint8_t>& raw) {
    if (raw.empty()) return {};
    std::vector<uint8_t> payload = encodeLz(raw);

    MemoryGuard mg;
    mg.add(payload.size() + 64);

    std::vector<uint8_t> container;
    container.reserve(4 + 1 + 3 + 8 + 8 + payload.size());
    container.push_back('M');
    container.push_back('K');
    container.push_back('P');
    container.push_back('1');
    container.push_back(3); // method: 3 = LZ backref stream
    container.push_back(0);
    container.push_back(0);
    container.push_back(0);
    writeU64LE(container, static_cast<uint64_t>(raw.size()));
    writeU64LE(container, static_cast<uint64_t>(payload.size()));
    container.insert(container.end(), payload.begin(), payload.end());

    if (container.size() >= raw.size()) return {};
    return container;
}

static std::vector<uint8_t> buildLzThenHuffmanCompressedPatchBlobIfSmaller(const std::vector<uint8_t>& raw) {
    if (raw.empty()) return {};
    const std::vector<uint8_t> lzPayload = encodeLz(raw);

    std::array<uint8_t, 256> lengths{};
    std::vector<uint8_t> encoded;
    if (!encodeHuffman(lzPayload, encoded, lengths)) return {};

    MemoryGuard mg;
    mg.add(lzPayload.size() + encoded.size() + 1024);

    std::vector<uint8_t> container;
    // MKP1 + method + pad + raw_size + payload_size + lz_size + lengths + payload
    container.reserve(4 + 1 + 3 + 8 + 8 + 8 + 256 + encoded.size());
    container.push_back('M');
    container.push_back('K');
    container.push_back('P');
    container.push_back('1');
    container.push_back(4); // method: 4 = LZ then Huffman
    container.push_back(0);
    container.push_back(0);
    container.push_back(0);
    writeU64LE(container, static_cast<uint64_t>(raw.size()));
    writeU64LE(container, static_cast<uint64_t>(encoded.size()));
    writeU64LE(container, static_cast<uint64_t>(lzPayload.size()));
    container.insert(container.end(), lengths.begin(), lengths.end());
    container.insert(container.end(), encoded.begin(), encoded.end());

    if (container.size() >= raw.size()) return {};
    return container;
}

} // namespace

void DiffEngine::emitStarPlusChunks(std::vector<PatchOperation>& localOperations,
                                      const std::string& relPath,
                                      size_t& cursor,
                                      size_t& addedBytes,
                                      const std::vector<uint8_t>& insertBytes) {
    for (size_t off = 0; off < insertBytes.size(); off += kStarPlusChunkBytes) {
        const size_t len = std::min(kStarPlusChunkBytes, insertBytes.size() - off);
        const uint8_t* chunkPtr = insertBytes.data() + off;

        size_t overlap = 0;
        size_t from = 0;
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            overlap = matchingSuffixPrefix(patchesBlob_, chunkPtr, len);
            if (overlap > 0) {
                from = patchesBlob_.size() - overlap;
            }
        }

        if (overlap > 0) {
            PatchOperation reuseOp;
            reuseOp.op = "*+";
            reuseOp.path = relPath;
            reuseOp.start_bytes = cursor;
            reuseOp.from_bytes = from;
            reuseOp.to_bytes = from + overlap;
            localOperations.push_back(reuseOp);
            cursor += overlap;
            addedBytes += overlap;
        }

        const size_t tail = len - overlap;
        if (tail == 0) {
            continue;
        }

        const size_t blobStart = appendToBlobWithDedup(chunkPtr + overlap, tail);

        PatchOperation addOp;
        addOp.op = "*+";
        addOp.path = relPath;
        addOp.start_bytes = cursor;
        addOp.from_bytes = blobStart;
        addOp.to_bytes = blobStart + tail;
        localOperations.push_back(addOp);

        cursor += tail;
        addedBytes += tail;
    }
}

DiffEngine::DiffEngine(const std::string& oldDir, const std::string& newDir, const std::string& outDir)
    : oldDir_(oldDir), newDir_(newDir), outDir_(outDir) {}

size_t DiffEngine::appendToBlob(const std::vector<uint8_t>& data) {
    {
        const size_t bytes = data.size();
        if (currentMemoryUsage_.fetch_add(bytes) + bytes > memoryLimitBytes_) {
            currentMemoryUsage_.fetch_sub(bytes);
            throw std::runtime_error("Patcher exceeded global memory limit while appending to blob.");
        }
    }
    std::lock_guard<std::mutex> lock(stateMutex_);
    const size_t start = patchesBlob_.size();
    patchesBlob_.insert(patchesBlob_.end(), data.begin(), data.end());
    return start;
}

size_t DiffEngine::appendToBlobWithDedup(const uint8_t* data, size_t len) {
    if (len == 0) {
        std::lock_guard<std::mutex> lock(stateMutex_);
        return patchesBlob_.size();
    }
    const uint64_t h = fnv1a64(data, len) ^ static_cast<uint64_t>(len);
    
    std::lock_guard<std::mutex> lock(stateMutex_);
    auto& candidates = patchBlobDedup_[h];
    for (size_t off : candidates) {
        if (off + len <= patchesBlob_.size() &&
            std::equal(data, data + len, patchesBlob_.begin() + static_cast<std::ptrdiff_t>(off))) {
            return off;
        }
    }

    {
        const size_t bytes = len;
        if (currentMemoryUsage_.fetch_add(bytes) + bytes > memoryLimitBytes_) {
            currentMemoryUsage_.fetch_sub(bytes);
            throw std::runtime_error("Patcher exceeded global memory limit while appending to blob.");
        }
    }
    const size_t start = patchesBlob_.size();
    patchesBlob_.insert(patchesBlob_.end(), data, data + len);
    candidates.push_back(start);
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
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        patchBlobDedup_.clear();
        operations_.clear();
        patchesBlob_.clear();
        currentMemoryUsage_ = 0;
    }

    nlohmann::json instructions;
    instructions["files"] = nlohmann::json::array();
    std::mutex instructionsMutex;

    std::vector<std::future<void>> futures;
    const size_t maxConcurrentTasks = std::max<size_t>(1, std::thread::hardware_concurrency());

    auto waitForSpace = [&]() {
        if (futures.size() >= maxConcurrentTasks) {
            for (auto it = futures.begin(); it != futures.end(); ) {
                if (it->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                    it->get();
                    it = futures.erase(it);
                } else {
                    ++it;
                }
            }
            if (futures.size() >= maxConcurrentTasks) {
                futures.front().get();
                futures.erase(futures.begin());
            }
        }
    };

    // Iterate through new directory to find new/modified files
    for (const auto& entry : fs::recursive_directory_iterator(newDir_)) {
        if (!entry.is_regular_file()) continue;
        
        waitForSpace();
        futures.push_back(std::async(std::launch::async, [this, &instructions, &instructionsMutex, entry]() {
            fs::path relPath = fs::relative(entry.path(), newDir_);
            fs::path oldPath = fs::path(oldDir_) / relPath;
            
            nlohmann::json fileJson = nlohmann::json::array();
            fileJson.push_back(relPath.generic_string());
            fileJson.push_back(hashFile(entry.path()));
            nlohmann::json opsJson = nlohmann::json::array();

            std::vector<PatchOperation> fileOps;
            if (!fs::exists(oldPath)) {
                fileOps = handleNewFile(entry.path(), relPath.generic_string());
            } else {
                fileOps = processFile(oldPath, entry.path(), relPath.generic_string());
            }

            if (!fileOps.empty()) {
                for (const auto& op : fileOps) {
                    opsJson.push_back(op.toCompactJson());
                }
                fileJson.push_back(opsJson);
                
                std::lock_guard<std::mutex> lock(instructionsMutex);
                instructions["files"].push_back(fileJson);
                
                std::lock_guard<std::mutex> stateLock(stateMutex_);
                operations_.insert(operations_.end(), fileOps.begin(), fileOps.end());
            }
        }));
    }

    // Iterate through old directory to find deleted files
    for (const auto& entry : fs::recursive_directory_iterator(oldDir_)) {
        if (!entry.is_regular_file()) continue;
        
        fs::path relPath = fs::relative(entry.path(), oldDir_);
        fs::path newPath = fs::path(newDir_) / relPath;
        
        if (!fs::exists(newPath)) {
            waitForSpace();
            futures.push_back(std::async(std::launch::async, [this, &instructions, &instructionsMutex, relPath]() {
                nlohmann::json fileJson = nlohmann::json::array();
                fileJson.push_back(relPath.generic_string());
                fileJson.push_back(""); // No hash for deleted files
                nlohmann::json opsJson = nlohmann::json::array();
                
                std::vector<PatchOperation> fileOps = handleDeletedFile(relPath.generic_string());
                if (!fileOps.empty()) {
                    for (const auto& op : fileOps) {
                        opsJson.push_back(op.toCompactJson());
                    }
                    fileJson.push_back(opsJson);
                    
                    std::lock_guard<std::mutex> lock(instructionsMutex);
                    instructions["files"].push_back(fileJson);
                    
                    std::lock_guard<std::mutex> stateLock(stateMutex_);
                    operations_.insert(operations_.end(), fileOps.begin(), fileOps.end());
                }
            }));
        }
    }

    // Wait for all remaining tasks
    for (auto& f : futures) {
        if (f.valid()) f.get();
    }

    // Write out patches.bin
    fs::path binPath = fs::path(outDir_) / "patches.bin";
    std::ofstream binOut(binPath, std::ios::binary);
    if (!binOut) {
        std::cerr << "Failed to open " << binPath << " for writing.\n";
        return false;
    }

    // Parallelize compression checks
    auto huffFuture = std::async(std::launch::async, [this]() { return buildCompressedPatchBlobIfSmaller(patchesBlob_); });
    auto rleFuture = std::async(std::launch::async, [this]() { return buildRleCompressedPatchBlobIfSmaller(patchesBlob_); });
    auto lzFuture = std::async(std::launch::async, [this]() { return buildLzCompressedPatchBlobIfSmaller(patchesBlob_); });
    auto lzHuffFuture = std::async(std::launch::async, [this]() { return buildLzThenHuffmanCompressedPatchBlobIfSmaller(patchesBlob_); });

    const std::vector<uint8_t> huffContainer = huffFuture.get();
    const std::vector<uint8_t> rleContainer = rleFuture.get();
    const std::vector<uint8_t> lzContainer = lzFuture.get();
    const std::vector<uint8_t> lzHuffContainer = lzHuffFuture.get();

    const std::vector<uint8_t>* bestContainer = nullptr;
    if (!huffContainer.empty()) bestContainer = &huffContainer;
    if (!rleContainer.empty() && (!bestContainer || rleContainer.size() < bestContainer->size())) {
        bestContainer = &rleContainer;
    }
    if (!lzContainer.empty() && (!bestContainer || lzContainer.size() < bestContainer->size())) {
        bestContainer = &lzContainer;
    }
    if (!lzHuffContainer.empty() && (!bestContainer || lzHuffContainer.size() < bestContainer->size())) {
        bestContainer = &lzHuffContainer;
    }

    if (bestContainer) {
        binOut.write(reinterpret_cast<const char*>(bestContainer->data()), bestContainer->size());
    } else {
        binOut.write(reinterpret_cast<const char*>(patchesBlob_.data()), patchesBlob_.size());
    }

    // Write out instructions.bin (MessagePack format)
    fs::path binPathInstructions = fs::path(outDir_) / "instructions.bin";
    std::ofstream binOutInstructions(binPathInstructions, std::ios::binary);
    if (!binOutInstructions) {
        std::cerr << "Failed to open " << binPathInstructions << " for writing.\n";
        return false;
    }
    std::vector<uint8_t> msgpack = nlohmann::json::to_msgpack(instructions);
    binOutInstructions.write(reinterpret_cast<const char*>(msgpack.data()), msgpack.size());

    return true;
}

std::vector<PatchOperation> DiffEngine::handleNewFile(const fs::path& newPath, const std::string& relPath) {
    auto data = readFile(newPath);
    
    MemoryGuard mg;
    mg.add(data.size());

    size_t start = appendToBlob(data);
    
    std::cout << "  [NEW] " << relPath << " (" << data.size() << " bytes)\n";

    PatchOperation op;
    op.op = "+";
    op.from_bytes = start;
    op.to_bytes = start + data.size();
    return {op};
}

std::vector<PatchOperation> DiffEngine::handleDeletedFile(const std::string& relPath) {
    std::cout << "  [DEL] " << relPath << "\n";
    
    PatchOperation op;
    op.op = "-";
    return {op};
}

std::vector<PatchOperation> DiffEngine::processFile(const fs::path& oldPath, const fs::path& newPath, const std::string& relPath) {
    auto oldData = readFile(oldPath);
    auto newData = readFile(newPath);

    MemoryGuard mg;
    mg.add(oldData.size() + newData.size());
    
    if (oldData == newData) {
        std::cout << "  [UNCHANGED] " << relPath << "\n";
        return {}; // Unchanged
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

    std::vector<PatchOperation> localOps;
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
                    localOps.push_back(removeOp);
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
                    emitStarPlusChunks(localOps, relPath, localCursor, addedBytes, insertBytes);
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
            localOps.push_back(removeOp);
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
            emitStarPlusChunks(localOps, relPath, cursor, addedBytes, insertBytes);
        }
    }

    std::cout << "  [MOD] " << relPath << " (-" << removedBytes << " bytes, +" << addedBytes << " bytes)"
              << (usedBlockMyers ? " [myers-block:" + std::to_string(selectedBlockSize) + "]" : " [myers-byte]")
              << (refinedSegments > 0 ? " [refined:" + std::to_string(refinedSegments) + "]" : "")
              << "\n";
              
    return localOps;
}

} // namespace patcher
