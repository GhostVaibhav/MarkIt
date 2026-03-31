#pragma once
#include <string>
#include <unordered_set>

#include "json.hpp"

struct SyncStatus {
  unsigned int pendingPushes = 0;
  unsigned int pendingPulls = 0;

  bool isInSync() const { return pendingPushes == 0 && pendingPulls == 0; }

  static SyncStatus compute(const nlohmann::json &localData,
                            const nlohmann::json &remoteData) {
    SyncStatus status;

    std::unordered_set<std::string> localDataSetIds, remoteDataSetIds;

    if (localData.contains("data")) {
      for (const auto &t : localData["data"]) {
        localDataSetIds.insert(t["id"]);
      }
    }

    if (remoteData.contains("data")) {
      for (const auto &t : remoteData["data"]) {
        remoteDataSetIds.insert(t["id"]);
      }
    }

    unsigned int push = 0, pull = 0;

    for (const auto &t : localDataSetIds) {
      if (remoteDataSetIds.find(t) == remoteDataSetIds.end()) {
        ++push;
      }
    }

    for (const auto &t : remoteDataSetIds) {
      if (localDataSetIds.find(t) == localDataSetIds.end()) {
        ++pull;
      }
    }

    status.pendingPulls = pull;
    status.pendingPushes = push;

    return status;
  }
};

enum class SyncResult { Success, AlreadyInSync, NetworkError };
