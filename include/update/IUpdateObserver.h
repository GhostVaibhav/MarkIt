#pragma once

#include "UpdateStatus.h"

#include <string>

/**
 * Observer interface for update status changes.
 * Follows the Interface Segregation Principle — only one method,
 * keeping it minimal and focused. Application implements this to
 * receive notifications from BackgroundUpdateService.
 */
class IUpdateObserver {
 public:
  virtual ~IUpdateObserver() = default;

  /**
   * Called by BackgroundUpdateService when the update status changes.
   * @param status  Current update lifecycle state
   * @param version Latest version string (empty if not yet known)
   */
  virtual void onUpdateStatusChanged(UpdateStatus status,
                                     const std::string& version) = 0;
};
