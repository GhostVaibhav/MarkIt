#pragma once

enum class SyncOperation {
  None = 0,
  Push = 1,
  Pull = 2,
  Refresh = 3,
  CheckUpdates = 4
};
