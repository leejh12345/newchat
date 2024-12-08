#include "stdafx.h" // 또는 #include "pch.h" - 프로젝트 설정에 따라 둘 중 하나를 사용
#include "DatabaseManager.h"
#include <iostream>
#include <sqlite3.h>

DatabaseManager::DatabaseManager() : db(nullptr) {}

DatabaseManager::~DatabaseManager() {
    closeDatabase();
}

bool DatabaseManager::openDatabase(const std::string& dbName) {
    int rc = sqlite3_open(dbName.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS ChatLog (ID INTEGER PRIMARY KEY AUTOINCREMENT, Message TEXT);";
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

void DatabaseManager::closeDatabase() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool DatabaseManager::saveMessage(const std::string& message) {
       
    const char* sql = "INSERT INTO ChatLog (Message) VALUES (?);";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, message.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}
