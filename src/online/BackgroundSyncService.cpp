#include "BackgroundSyncService.h"
#include <spdlog/spdlog.h>
#include "config/UpdateConfig.h"

BackgroundSyncService::BackgroundSyncService() {}

BackgroundSyncService::~BackgroundSyncService() { stop(); }

void BackgroundSyncService::start(SyncManager* syncMgr, TodoManager* todoMgr,
                                   std::function<std::string()> getUserId,
                                   std::function<std::string()> getUserHash) {
  stop();  // ensure any previous thread is cleaned up

  syncManager = syncMgr;
  todoManager = todoMgr;
  getIdFn = std::move(getUserId);
  getHashFn = std::move(getUserHash);

  running = true;
  enabled = true;
  bgThread = std::thread(&BackgroundSyncService::worker, this);
  spdlog::info("BackgroundSyncService: started (interval={}s)", UpdateConfig::kBgSyncIntervalSeconds);
}

void BackgroundSyncService::stop() {
  if (!running) return;
  running = false;
  cv.notify_all();
  if (bgThread.joinable()) bgThread.join();
  spdlog::info("BackgroundSyncService: stopped");
}

bool BackgroundSyncService::toggle() {
  enabled = !enabled;
  spdlog::info("BackgroundSyncService: auto-sync {}", enabled ? "enabled" : "disabled");
  cv.notify_all();
  return enabled;
}

bool BackgroundSyncService::isEnabled() const { return enabled; }

std::mutex& BackgroundSyncService::getSyncMutex() { return syncMtx; }

void BackgroundSyncService::worker() {
  while (running) {
    // Sleep for the poll interval, but wake up early on stop/toggle
    {
      std::unique_lock<std::mutex> lk(syncMtx);
      cv.wait_for(lk, std::chrono::seconds(UpdateConfig::kBgSyncIntervalSeconds),
                  [this] { return !running.load(); });
    }

    if (!running) break;
    if (!enabled) continue;

    // 1. Fetch info under lock (quick)
    std::string userId;
    std::string userHash;
    std::vector<Todo> todos;
    {
      std::lock_guard<std::mutex> lk(syncMtx);
      userId = getIdFn ? getIdFn() : "";
      userHash = getHashFn ? getHashFn() : "";
      if (syncManager && todoManager) {
        todos = todoManager->getAllTodos();
      }
    }

    if (userId.empty()) continue;

    // 2. Perform IO UNLOCKED (slow network call)
    BucketResult fetched;
    try {
        fetched = syncManager->getFacade().loadBucket(userId);
    } catch (const std::exception& e) {
        spdlog::error("BackgroundSyncService: IO failed: {}", e.what());
        continue;
    }

    if (!fetched.ok && !fetched.notFound) {
        spdlog::warn("BackgroundSyncService: loadBucket failed (network error), skipping refresh");
        continue;
    }
    if (fetched.notFound) {
        spdlog::warn("BackgroundSyncService: Remote bucket is absent or expired — skipping refresh. "
                     "User should push to recreate the bucket.");
        continue;
    }

    // 3. Apply update under lock (quick) — triggers notifyObservers()
    {
      std::lock_guard<std::mutex> lk(syncMtx);
      if (syncManager) {
        syncManager->applyRemoteUpdate(fetched.data, userId, userHash, todos);
        spdlog::info("BackgroundSyncService: refresh completed");
      }
    }
    // Observer notification (from applyRemoteUpdate) wakes the UI via CV
  }
}
