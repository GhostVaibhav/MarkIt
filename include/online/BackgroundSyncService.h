#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <chrono>

#include "SyncManager.h"
#include "TodoManager.h"

/// Encapsulates the background thread that periodically refreshes sync status.
/// Completely self-contained — Application only needs to call start/stop/toggle.
class BackgroundSyncService {
 public:
  BackgroundSyncService();
  ~BackgroundSyncService();

  // Non-copyable
  BackgroundSyncService(const BackgroundSyncService&) = delete;
  BackgroundSyncService& operator=(const BackgroundSyncService&) = delete;

  /// Start the background polling thread.
  /// @param syncMgr   Pointer to the active SyncManager (must outlive the service).
  /// @param todoMgr   Pointer to the active TodoManager.
  /// @param getUserId Callback returning current user id (empty = not logged in).
  /// @param getUserHash Callback returning current user password hash.
  void start(SyncManager* syncMgr, TodoManager* todoMgr,
             std::function<std::string()> getUserId,
             std::function<std::string()> getUserHash);

  /// Gracefully stop the background thread. Safe to call multiple times.
  void stop();

  /// Toggle auto-sync on/off. Returns the new state.
  bool toggle();

  /// Query whether auto-sync is currently enabled.
  bool isEnabled() const;

  /// Mutex that protects SyncManager access. Application must lock this
  /// before any foreground SyncManager call (push/pull/refresh/recompute).
  std::mutex& getSyncMutex();

 private:
  void worker();

  std::thread bgThread;
  std::mutex syncMtx;
  std::condition_variable cv;
  std::atomic<bool> running{false};
  std::atomic<bool> enabled{true};

  SyncManager* syncManager = nullptr;
  TodoManager* todoManager = nullptr;
  std::function<std::string()> getIdFn;
  std::function<std::string()> getHashFn;

  static constexpr int POLL_INTERVAL_SECONDS = 10;
};
