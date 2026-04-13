#include "BackgroundSyncService.h"

#include <spdlog/spdlog.h>

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
  spdlog::info("BackgroundSyncService: started (interval={}s)", POLL_INTERVAL_SECONDS);
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
  // Wake up the worker so it can check the new state immediately
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
      cv.wait_for(lk, std::chrono::seconds(POLL_INTERVAL_SECONDS),
                  [this] { return !running.load(); });
    }

    if (!running) break;
    if (!enabled) continue;

    // Check preconditions
    std::string userId = getIdFn ? getIdFn() : "";
    std::string userHash = getHashFn ? getHashFn() : "";
    if (userId.empty()) continue;

    // Perform the refresh under lock
    {
      std::lock_guard<std::mutex> lk(syncMtx);
      if (syncManager && todoManager) {
        try {
          auto todos = todoManager->getAllTodos();
          syncManager->refreshData(userId, userHash, todos);
          spdlog::info("BackgroundSyncService: refresh completed");
        } catch (const std::exception& e) {
          spdlog::error("BackgroundSyncService: refresh failed: {}", e.what());
        }
      }
    }
  }
}
