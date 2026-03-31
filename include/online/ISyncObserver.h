#pragma once
#include "SyncStatus.h"

class ISyncObserver {
 public:
  virtual ~ISyncObserver() = default;
  virtual void onSyncStatusChanged(const SyncStatus& status) = 0;
};
