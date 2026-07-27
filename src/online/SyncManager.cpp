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

SyncStatus SyncManager::getSyncStatus() const {
  std::lock_guard<std::mutex> lock(statusMtx);
  return syncStatus;
}

SyncResult SyncManager::pull(const std::string& userId,
                             nlohmann::json& localData) {
  spdlog::info("SyncManager: Initiating cloud data pull for user '{}'", userId);
  BucketResult fetched = pantryFacade.loadBucket(userId);
  if (!fetched.ok && !fetched.notFound) {
    spdlog::error("SyncManager: Cloud pull failed (network error)");
    return SyncResult::NetworkError;
  }

  // notFound (404) means the bucket has expired or never existed.
  // Return BucketExpired so the caller can prompt the user to push-to-recreate.
  nlohmann::json remoteData = std::move(fetched.data);
  if (fetched.notFound) {
    spdlog::warn("SyncManager: Remote bucket is absent or expired — returning BucketExpired");
    return SyncResult::BucketExpired;
  }
  if (remoteData.empty()) {
    spdlog::info("SyncManager: Remote bucket is empty — skipping pull");
    return SyncResult::AlreadyInSync;
  }

  {
    std::lock_guard<std::mutex> lock(statusMtx);
    syncStatus = SyncStatus::compute(localData, remoteData);
    if (syncStatus.pendingPulls == 0) {
      cachedRemoteData = remoteData;
      spdlog::info(
          "SyncManager: Pull skipped (dataset already vertically synced)");
      return SyncResult::AlreadyInSync;
    }

    if (localData.empty() || !localData.contains("data")) {
      localData = remoteData;
    } else {
      std::unordered_set<std::string> localIds;
      for (const auto& t : localData["data"]) {
        localIds.insert(t["id"].get<std::string>());
      }
      if (remoteData.contains("data")) {
        for (const auto& t : remoteData["data"]) {
          if (localIds.find(t["id"].get<std::string>()) == localIds.end()) {
            localData["data"].push_back(t);
          }
        }
      }
    }

    cachedRemoteData = remoteData;
    syncStatus.pendingPulls = 0;
  }
  notifyObservers();
  spdlog::info("SyncManager: Successfully merged cloud data locally");
  return SyncResult::Success;
}

SyncResult SyncManager::push(const std::string& userId,
                             const nlohmann::json& localData) {
  spdlog::info("SyncManager: Initiating cloud data push for user '{}'", userId);
  BucketResult fetched = pantryFacade.loadBucket(userId);

  if (!fetched.ok && !fetched.notFound) {
    // Preflight failed with a network error. This may be transient (e.g. a
    // brief connectivity blip). Log a warning but still attempt the save —
    // the PUT itself will reveal whether the server is reachable.
    spdlog::warn(
        "SyncManager: Preflight loadBucket failed (network error) — "
        "skipping in-sync check and attempting push anyway");
  } else if (fetched.notFound) {
    // notFound (404 or 400 "does not exist") means the bucket has expired or
    // never existed. We already confirmed it's absent from the preflight —
    // call createBucket() directly to avoid a redundant bucketExists GET.
    spdlog::warn("SyncManager: Remote bucket absent/expired — recreating before push");
    if (!pantryFacade.createBucket(userId)) {
      spdlog::error("SyncManager: Failed to recreate bucket — aborting push");
      return SyncResult::BucketExpired;
    }
    spdlog::info("SyncManager: Bucket recreated — proceeding with push");
  } else {
    // Preflight succeeded — check if a push is actually needed.
    nlohmann::json remoteData = std::move(fetched.data);
    if (!remoteData.empty()) {
      std::lock_guard<std::mutex> lock(statusMtx);
      syncStatus = SyncStatus::compute(localData, remoteData);
      if (syncStatus.pendingPushes == 0) {
        cachedRemoteData = remoteData;
        spdlog::info("SyncManager: Push skipped (cloud already vertically synced)");
        return SyncResult::AlreadyInSync;
      }
    }
  }

  if (pantryFacade.saveBucket(userId, localData)) {
    {
      std::lock_guard<std::mutex> lock(statusMtx);
      cachedRemoteData = localData;
      syncStatus.pendingPushes = 0;
    }
    notifyObservers();
    spdlog::info("SyncManager: Successfully pushed local data to cloud");
    return SyncResult::Success;
  }

  spdlog::error("SyncManager: Failed to push local data to cloud (save fail)");
  return SyncResult::NetworkError;
}

SyncStatus SyncManager::refresh(const std::string& userId,
                                const nlohmann::json& localData) {
  BucketResult fetched = pantryFacade.loadBucket(userId);
  if (fetched.ok && !fetched.data.empty()) {
    SyncStatus statusCpy;
    {
      std::lock_guard<std::mutex> lock(statusMtx);
      cachedRemoteData = fetched.data;
      syncStatus = SyncStatus::compute(localData, fetched.data);
      statusCpy = syncStatus;
    }
    spdlog::info(
        "SyncManager: Refreshed cloud sync status: {} pull(s), {} push(es) "
        "pending",
        statusCpy.pendingPulls, statusCpy.pendingPushes);
    notifyObservers();
  } else if (fetched.notFound || (fetched.ok && fetched.data.empty())) {
    spdlog::info("SyncManager: Cloud refresh: remote bucket is absent or empty (new user)");
  } else {
    spdlog::warn("SyncManager: Cloud refresh failed (network error)");
  }
  return getSyncStatus();
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
  localData["data"] = nlohmann::json::array();
  for (const auto& t : outTodos) {
    nlohmann::json tJson;
    tJson["id"] = t.id;
    tJson["name"] = t.name;
    tJson["desc"] = t.desc;
    tJson["time"] = t.time;
    tJson["isComplete"] = t.isComplete;
    localData["data"].push_back(tJson);
  }

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

void SyncManager::applyRemoteUpdate(const nlohmann::json& remoteData,
                                     const std::string& userId,
                                     const std::string& hash,
                                     const std::vector<Todo>& todos) {
  (void)userId;
  if (remoteData.empty()) return;

  {
    std::lock_guard<std::mutex> lock(statusMtx);
    cachedRemoteData = remoteData;
  }

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

  SyncStatus statusCpy;
  {
    std::lock_guard<std::mutex> lock(statusMtx);
    syncStatus = SyncStatus::compute(localData, remoteData);
    statusCpy = syncStatus;
  }
  spdlog::info(
      "SyncManager: Applied remote update locally: {} pull(s), {} push(es) "
      "pending",
      statusCpy.pendingPulls, statusCpy.pendingPushes);
  notifyObservers();
}

void SyncManager::recomputeData(const std::vector<Todo>& todos) {
  {
    std::lock_guard<std::mutex> lock(statusMtx);
    if (cachedRemoteData.empty()) return;
  }
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

  {
    std::lock_guard<std::mutex> lock(statusMtx);
    syncStatus = SyncStatus::compute(localData, cachedRemoteData);
  }
  notifyObservers();
}
