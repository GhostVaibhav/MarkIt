#pragma once
#include <vector>
#include <mutex>
#include <functional>

#include "ISyncObserver.h"
#include "PantryFacade.h"
#include "SyncStatus.h"
#include "Todo.h"

class SyncManager {
 public:
  explicit SyncManager(const PantryFacade& facade);

  SyncResult pull(const std::string& userId, nlohmann::json& localData);
  SyncResult push(const std::string& userId, const nlohmann::json& localData);
  SyncStatus refresh(const std::string& userId,
                     const nlohmann::json& localData);

  SyncResult pushData(const std::string& userId, const std::string& hash,
                      const std::vector<Todo>& todos);
  SyncResult pullData(const std::string& userId, std::vector<Todo>& outTodos);
  SyncStatus refreshData(const std::string& userId, const std::string& hash,
                         const std::vector<Todo>& todos);
  void applyRemoteUpdate(const nlohmann::json& remoteData, const std::string& userId,
                         const std::string& hash, const std::vector<Todo>& todos);
  void recomputeData(const std::vector<Todo>& todos);

  void setRemoteCache(const std::string& cacheData);
  void setCacheCallback(std::function<void(const std::string&, const std::string&)> saveCb);

  SyncStatus getSyncStatus() const;
  const PantryFacade& getFacade() const { return pantryFacade; }
  void addObserver(ISyncObserver* observer);

 private:
  nlohmann::json cachedRemoteData;
  PantryFacade pantryFacade;
  SyncStatus syncStatus;
  mutable std::mutex statusMtx;
  std::vector<ISyncObserver*> observers;
  std::function<void(const std::string&, const std::string&)> saveCacheCallback;

  void notifyObservers();
};
