#include "SyncManager.h"

#include <spdlog/spdlog.h>

SyncManager::SyncManager(const PantryFacade& facade) : pantryFacade(facade) {}

void SyncManager::addObserver(ISyncObserver* observer) {
  if (observer) observers.push_back(observer);
}

void SyncManager::notifyObservers() {
  for (auto* obs : observers) {
    obs->onSyncStatusChanged(syncStatus);
  }
}

SyncStatus SyncManager::getSyncStatus() const { return syncStatus; }

SyncResult SyncManager::pull(const std::string& userId,
                             nlohmann::json& localData) {
  spdlog::info("SyncManager: Initiating cloud data pull for user '{}'", userId);
  nlohmann::json remoteData = pantryFacade.loadBucket(userId);
  if (remoteData.empty()) {
    spdlog::error("SyncManager: Cloud pull failed (network or bucket empty)");
    return SyncResult::NetworkError;
  }

  syncStatus = SyncStatus::compute(localData, remoteData);
  if (syncStatus.pendingPulls == 0) {
    cachedRemoteData = remoteData;
    spdlog::info(
        "SyncManager: Pull skipped (dataset already vertically synced)");
    return SyncResult::AlreadyInSync;
  }

  localData = remoteData;
  cachedRemoteData = remoteData;
  syncStatus.pendingPulls = 0;
  notifyObservers();
  spdlog::info("SyncManager: Successfully merged cloud data locally");
  return SyncResult::Success;
}

SyncResult SyncManager::push(const std::string& userId,
                             const nlohmann::json& localData) {
  spdlog::info("SyncManager: Initiating cloud data push for user '{}'", userId);
  nlohmann::json remoteData = pantryFacade.loadBucket(userId);
  if (remoteData.empty()) {
    spdlog::error(
        "SyncManager: Cloud push validation failed (network or bucket empty)");
    return SyncResult::NetworkError;
  }

  syncStatus = SyncStatus::compute(localData, remoteData);
  if (syncStatus.pendingPushes == 0) {
    cachedRemoteData = remoteData;
    spdlog::info("SyncManager: Push skipped (cloud already vertically synced)");
    return SyncResult::AlreadyInSync;
  }

  if (pantryFacade.saveBucket(userId, localData)) {
    cachedRemoteData = localData;
    syncStatus.pendingPushes = 0;
    notifyObservers();
    spdlog::info("SyncManager: Successfully pushed local data to cloud");
    return SyncResult::Success;
  }

  spdlog::error("SyncManager: Failed to push local data to cloud (save fail)");
  return SyncResult::NetworkError;
}

SyncStatus SyncManager::refresh(const std::string& userId,
                                const nlohmann::json& localData) {
  nlohmann::json remoteData = pantryFacade.loadBucket(userId);
  if (!remoteData.empty()) {
    cachedRemoteData = remoteData;
    syncStatus = SyncStatus::compute(localData, remoteData);
    spdlog::info(
        "SyncManager: Refreshed cloud sync status: {} pull(s), {} push(es) "
        "pending",
        syncStatus.pendingPulls, syncStatus.pendingPushes);
    notifyObservers();
  } else {
    spdlog::warn("remoteData: {}", remoteData.dump());
    spdlog::warn("SyncManager: Cloud refresh failed (network or bucket empty)");
  }
  return syncStatus;
}

SyncResult SyncManager::pushData(const std::string& userId,
                                 const std::string& hash,
                                 const std::vector<Todo>& todos) {
  nlohmann::json localData;
  localData["hash"] = hash;
  localData["number"] = 0;

  localData["data"] = nlohmann::json::array();
  for (const auto& t : todos) {
    nlohmann::json tJson;
    tJson["id"] = t.id;
    tJson["name"] = t.name;
    tJson["desc"] = t.desc;
    tJson["time"] = t.time;
    tJson["isComplete"] = t.isComplete;
    localData["data"].push_back(tJson);
  }

  return push(userId, localData);
}

SyncResult SyncManager::pullData(const std::string& userId,
                                 std::vector<Todo>& outTodos) {
  nlohmann::json localData;
  SyncResult result = pull(userId, localData);

  if (result == SyncResult::Success && localData.contains("data") &&
      localData["data"].is_array()) {
    outTodos.clear();
    for (const auto& tJson : localData["data"]) {
      Todo t;
      t.id = tJson.value("id", "");
      t.name = tJson.value("name", "");
      t.desc = tJson.value("desc", "");
      t.time = tJson.value("time", 0);
      t.isComplete = tJson.value("isComplete", false);
      outTodos.push_back(t);
    }
  }
  return result;
}

SyncStatus SyncManager::refreshData(const std::string& userId,
                                    const std::string& hash,
                                    const std::vector<Todo>& todos) {
  nlohmann::json localData;
  localData["hash"] = hash;
  localData["number"] = 0;

  localData["data"] = nlohmann::json::array();
  for (const auto& t : todos) {
    nlohmann::json tJson;
    tJson["id"] = t.id;
    tJson["name"] = t.name;
    tJson["desc"] = t.desc;
    tJson["time"] = t.time;
    tJson["isComplete"] = t.isComplete;
    localData["data"].push_back(tJson);
  }
  return refresh(userId, localData);
}

void SyncManager::recomputeData(const std::vector<Todo>& todos) {
  if (cachedRemoteData.empty()) return;
  nlohmann::json localData;
  localData["hash"] = "";
  localData["number"] = 0;
  localData["data"] = nlohmann::json::array();

  for (const auto& t : todos) {
    nlohmann::json tJson;
    tJson["id"] = t.id;
    tJson["name"] = t.name;
    tJson["desc"] = t.desc;
    tJson["time"] = t.time;
    tJson["isComplete"] = t.isComplete;
    localData["data"].push_back(tJson);
  }

  syncStatus = SyncStatus::compute(localData, cachedRemoteData);
  notifyObservers();
}
