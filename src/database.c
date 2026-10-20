#include "database.h"
#include <sqlite3.h>
#include <stdio.h>

int create_conn(sqlite3 **db) {
    int rc = sqlite3_open("tasks.db", db); // Opening the db

    // Checking for connection
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(*db));
        *db = NULL;
        return 1;
    }

    // Initial script
    const char *sql = "CREATE TABLE IF NOT EXISTS tasks ("
                      "id INTEGER PRIMARY KEY,"
                      "title VARCHAR(255) NOT NULL,"
                      "done INTEGER NOT NULL DEFAULT 0"
                      ");";

    rc = sqlite3_exec(*db, sql, NULL, NULL, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(*db));
        sqlite3_close(*db);
        *db = NULL;
        return 1;
    }

    return 0;
}
