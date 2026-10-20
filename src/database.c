#include "database.h"
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

int list_tasks(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL;

    const char *sql = "SELECT id, title, done FROM tasks ORDER BY id";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Prepare error: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *title = sqlite3_column_text(stmt, 1);
        int done = sqlite3_column_int(stmt, 2);

        printf("%d. [%c] %s\n", id, done ? 'x' : ' ', title);
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Query error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int add_task(sqlite3 *db, const char *title) {
    char *sql = sqlite3_mprintf("INSERT INTO tasks(title) VALUES(%Q);", title);

    if (sql == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    int rc = sqlite3_exec(db, sql, NULL, NULL, NULL);

    sqlite3_free(sql);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    printf("Task added successfully\n");
    return 0;
}

int complete_task(sqlite3 *db, int id) {
    char *sql = sqlite3_mprintf("UPDATE tasks SET done = 1 WHERE id = %d;", id);

    if (sql == NULL) {
        fprintf(stderr, "Memory allocation failed");
        return 1;
    }

    int rc = sqlite3_exec(db, sql, NULL, NULL, NULL);

    sqlite3_free(sql);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Task %d not found\n", id);
        return 1;
    }

    printf("Done task %d\n", id);
    return 0;
}
