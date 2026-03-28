#include "DBManager.h"

DBManager::DBManager(const std::string& dbPath) 
    : db(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {}
