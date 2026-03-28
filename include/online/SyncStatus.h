#pragma once
#include "json.hpp"

struct SyncStatus {
    int pendingPushes = 0;
    int pendingPulls = 0;

    bool isInSync() const { return pendingPushes == 0 && pendingPulls == 0; }

    static SyncStatus compute(const nlohmann::json& localData, const nlohmann::json& remoteData) {
        SyncStatus status;
        if (!localData.contains("number") || !remoteData.contains("number")) {
            return status;
        }
        int lNum = localData["number"];
        int rNum = remoteData["number"];
        
        if (lNum > rNum) {
            status.pendingPushes = lNum - rNum;
        } else if (rNum > lNum) {
            status.pendingPulls = rNum - lNum;
        }
        return status;
    }
};

enum class SyncResult {
    Success,
    AlreadyInSync,
    NetworkError
};
