#pragma once
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>

class DBManager {
public:
    explicit DBManager(const std::string& dbPath);
    virtual ~DBManager() = default;

protected:
    SQLite::Database db;
};
