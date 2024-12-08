#pragma once
#include "sqlite3.h"
#include <string>

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool openDatabase(const std::string& dbName);
    void closeDatabase();
    bool saveMessage(const std::string& message);

private:
    sqlite3* db;
};
