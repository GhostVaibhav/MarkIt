#include "BackgroundUpdateService.h"

#include <spdlog/spdlog.h>

#include "config/UpdateConfig.h"

BackgroundUpdateService::~BackgroundUpdateService() { stop(); }

void BackgroundUpdateService::start(const std::string& currentVersion) {
  if (running_) return;

  currentVersion_ = currentVersion;
  running_ = true;

  thread_ = std::thread(&BackgroundUpdateService::run, this);
  spdlog::info("BackgroundUpdateService: Started (version={}, interval={}min)",
               currentVersion, UpdateConfig::kCheckIntervalMinutes);
}

void BackgroundUpdateService::stop() {
  if (!running_) return;

  running_ = false;
  cv_.notify_all();

  if (thread_.joinable()) {
    thread_.join();
  }
  spdlog::info("BackgroundUpdateService: Stopped");
}

void BackgroundUpdateService::triggerCheck() {
  // Wake the thread to perform an immediate check
  cv_.notify_all();
}

void BackgroundUpdateService::run() {
  // Perform an immediate check on startup
  while (running_) {
    // --- Check phase ---
    {
      std::lock_guard<std::mutex> lock(mutex_);
      status_ = UpdateStatus::Checking;
    }
    notifyObservers(UpdateStatus::Checking, "");

    UpdateInfo result = checker_.checkForUpdate(currentVersion_);

    if (!running_) break;

    if (!result.updateAvailable) {
      {
        std::lock_guard<std::mutex> lock(mutex_);
        status_ = UpdateStatus::UpToDate;
        updateInfo_ = result;
      }
      notifyObservers(UpdateStatus::UpToDate, result.latestVersion);
    } else {
      // --- Download phase ---
      {
        std::lock_guard<std::mutex> lock(mutex_);
        status_ = UpdateStatus::Downloading;
        updateInfo_ = result;
      }
      notifyObservers(UpdateStatus::Downloading, result.latestVersion);

      bool success =
          downloader_.download(result.assetsToDownload, result.isFullUpdate);

      if (!running_) break;

      UpdateStatus finalStatus;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (success) {
          status_ = UpdateStatus::Ready;
          finalStatus = UpdateStatus::Ready;
          spdlog::info(
              "BackgroundUpdateService: Update {} staged and ready to apply",
              result.latestVersion);
        } else {
          status_ = UpdateStatus::Failed;
          finalStatus = UpdateStatus::Failed;
          spdlog::error(
              "BackgroundUpdateService: Failed to download update {}",
              result.latestVersion);
        }
      }
      notifyObservers(finalStatus, result.latestVersion);

      // Once an update is ready, stop checking
      if (success) return;
    }

    // Wait for the configured interval or until woken by triggerCheck()
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait_for(lock,
                   std::chrono::minutes(UpdateConfig::kCheckIntervalMinutes),
                   [this] { return !running_; });
    }
  }
}

UpdateStatus BackgroundUpdateService::getStatus() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return status_;
}

UpdateInfo BackgroundUpdateService::getUpdateInfo() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return updateInfo_;
}

bool BackgroundUpdateService::isUpdateReady() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return status_ == UpdateStatus::Ready && downloader_.isReady();
}

std::string BackgroundUpdateService::getStagedBinaryPath() const {
  // We now return the staging directory itself because the updater will process 
  // all patch files (or a single full release file) found within it.
  return UpdateConfig::getStagingDir();
}

void BackgroundUpdateService::addObserver(IUpdateObserver* observer) {
  std::lock_guard<std::mutex> lock(mutex_);
  observers_.push_back(observer);
}

void BackgroundUpdateService::removeObserver(IUpdateObserver* observer) {
  std::lock_guard<std::mutex> lock(mutex_);
  observers_.erase(
      std::remove(observers_.begin(), observers_.end(), observer),
      observers_.end());
}

void BackgroundUpdateService::notifyObservers(UpdateStatus status,
                                              const std::string& version) {
  // Note: called with mutex_ NOT held to prevent deadlock
  // (observers may call getStatus() which acquires the lock)
  std::vector<IUpdateObserver*> obs;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    obs = observers_;
  }
  for (auto* o : obs) {
    o->onUpdateStatusChanged(status, version);
  }
}
