#pragma once
#include <string>
#include <unordered_set>

#include "json.hpp"

struct SyncStatus {
  unsigned int pendingPushes = 0;
  unsigned int pendingPulls = 0;

  std::unordered_set<std::string> newLocalIds;
  std::unordered_set<std::string> modifiedLocalIds;

  bool isInSync() const { return pendingPushes == 0 && pendingPulls == 0; }

  static SyncStatus compute(const nlohmann::json &localData,
                            const nlohmann::json &remoteData,
                            const nlohmann::json &cachedRemoteData = nlohmann::json()) {
    SyncStatus status;
    std::unordered_map<std::string, nlohmann::json> remoteMap;

    if (remoteData.contains("data")) {
      for (const auto &t : remoteData["data"]) {
        remoteMap[t.value("id", "")] = t;
      }
    }

    if (localData.contains("data")) {
      for (const auto &t : localData["data"]) {
        std::string id = t.value("id", "");
        auto it = remoteMap.find(id);
        if (it == remoteMap.end()) {
          status.newLocalIds.insert(id);
        } else {
          // Check for modifications against the remote baseline
          if (t.value("name", "") != it->second.value("name", "") ||
              t.value("desc", "") != it->second.value("desc", "") ||
              t.value("isComplete", false) != it->second.value("isComplete", false)) {
            status.modifiedLocalIds.insert(id);
          }
        }
      }
    }

    status.pendingPushes = status.newLocalIds.size() + status.modifiedLocalIds.size();

    // Pulls are remote items that don't exist locally at all
    std::unordered_set<std::string> localIds;
    if (localData.contains("data")) {
      for (const auto &t : localData["data"]) {
        localIds.insert(t.value("id", ""));
      }
    }

    std::unordered_set<std::string> cachedIds;
    if (cachedRemoteData.contains("data")) {
      for (const auto &t : cachedRemoteData["data"]) {
        cachedIds.insert(t.value("id", ""));
      }
    }

    unsigned int pullCount = 0;
    unsigned int localDeleteCount = 0;
    for (const auto &pair : remoteMap) {
      if (localIds.find(pair.first) == localIds.end()) {
        if (!cachedIds.empty() && cachedIds.find(pair.first) != cachedIds.end()) {
          // Present in remote and cache, but missing in local -> locally deleted
          localDeleteCount++;
        } else {
          // Not in local and not in cache -> new from remote
          pullCount++;
        }
      }
    }
    status.pendingPulls = pullCount;
    // We add localDeleteCount to pendingPushes so the UI knows we have deletions to push
    status.pendingPushes += localDeleteCount;

    return status;
  }
};

enum class SyncResult { Success, AlreadyInSync, NetworkError, BucketExpired };
