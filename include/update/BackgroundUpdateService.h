#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "IUpdateObserver.h"
#include "UpdateChecker.h"
#include "UpdateDownloader.h"
#include "UpdateStatus.h"

/**
 * Orchestrates periodic update checking and downloading in a background thread.
 *
 * Single Responsibility: coordinates the check→download pipeline.
 * Dependency Inversion: notifies the Application through IUpdateObserver.
 * Open/Closed: the observer list can grow without modifying this class.
 */
class BackgroundUpdateService {
 public:
  BackgroundUpdateService() = default;
  ~BackgroundUpdateService();

  // Non-copyable, non-movable (owns a thread)
  BackgroundUpdateService(const BackgroundUpdateService&) = delete;
  BackgroundUpdateService& operator=(const BackgroundUpdateService&) = delete;

  /**
   * Starts the background thread that periodically checks for updates.
   * @param currentVersion The running app version (e.g. "0.1.0")
   */
  void start(const std::string& currentVersion);

  /**
   * Stops the background thread gracefully.
   */
  void stop();

  /**
   * Triggers an immediate (manual) update check on the background thread.
   */
  void triggerCheck();

  // --- Thread-safe getters ---

  UpdateStatus getStatus() const;
  UpdateInfo   getUpdateInfo() const;
  bool         isUpdateReady() const;
  std::string  getStagedBinaryPath() const;

  // --- Observer management ---

  void addObserver(IUpdateObserver* observer);
  void removeObserver(IUpdateObserver* observer);

 private:
  void run();
  void notifyObservers(UpdateStatus status, const std::string& version);

  UpdateChecker    checker_;
  UpdateDownloader downloader_;

  std::thread      thread_;
  std::atomic<bool> running_{false};

  mutable std::mutex mutex_;
  std::condition_variable cv_;

  UpdateStatus status_ = UpdateStatus::None;
  UpdateInfo   updateInfo_;
  std::string  currentVersion_;

  std::vector<IUpdateObserver*> observers_;
};
