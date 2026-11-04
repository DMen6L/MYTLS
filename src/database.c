#include "database.h"
#include "task.h"

#include <curses.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int create_conn(sqlite3 **db) {
    int rc = sqlite3_open("tasks.db", db); // Opening the db

    // Checking for connection
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(*db));
        sqlite3_close(*db);
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
        fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(*db));
        sqlite3_close(*db);
        *db = NULL;
        return 1;
    }

    return 0;
}

int fetch_tasks(sqlite3 *db, int page, size_t page_size, TaskList *out) {
    if (db == NULL || out == NULL || out->items != NULL || out->count != 0) {
        return 1;
    }

    if (page < 0 || page_size == 0 || page_size > SIZE_MAX / sizeof(Task) ||
        (uintmax_t)page_size > INT64_MAX) {
        return 1;
    }

    sqlite3_int64 limit = (sqlite3_int64)page_size;

    if (page != 0 && limit > INT64_MAX / page) {
        return 1;
    }

    sqlite3_int64 offset = (sqlite3_int64)page * limit;
    sqlite3_stmt *stmt = NULL;
    Task *items = NULL;
    size_t count = 0;

    const char *sql = "SELECT id, title, done FROM tasks "
                      "ORDER BY id LIMIT ? OFFSET ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        goto fail;
    }

    if (sqlite3_bind_int64(stmt, 1, limit) != SQLITE_OK ||
        sqlite3_bind_int64(stmt, 2, offset) != SQLITE_OK) {
        goto fail;
    }

    items = malloc(page_size * sizeof(*items));
    if (items == NULL) {
        goto fail;
    }

    int rc;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (count >= page_size) {
            goto fail;
        }

        Task *task = &items[count];
        task->id = sqlite3_column_int(stmt, 0);

        const unsigned char *title = sqlite3_column_text(stmt, 1);
        if (title == NULL) {
            goto fail;
        }

        snprintf(task->title, sizeof(task->title), "%s", (const char *)title);

        task->done = sqlite3_column_int(stmt, 2);
        count++;
    }

    if (rc != SQLITE_DONE) {
        goto fail;
    }

    int finalize_rc = sqlite3_finalize(stmt);
    stmt = NULL;

    if (finalize_rc != SQLITE_OK) {
        goto fail;
    }

    if (count == 0) {
        free(items);
        items = NULL;
    }

    out->items = items;
    out->count = count;
    return 0;

fail:
    sqlite3_finalize(stmt);
    free(items);
    return 1;
}

void task_list_free(TaskList *list) {
    if (list == NULL) {
        return;
    }

    free(list->items);
    list->items = NULL;
    list->count = 0;
}

int set_task_done(sqlite3 *db, int task_id, int done) {
    const char *sql = "UPDATE tasks SET done = ? WHERE id = ?;";

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        return 1;
    }

    sqlite3_bind_int(stmt, 1, done);
    sqlite3_bind_int(stmt, 2, task_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int add_task(sqlite3 *db, Task *new_task) {
    const char *sql = "INSERT INTO tasks (title) VALUES(?);";

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        return 1;
    }

    sqlite3_bind_text(stmt, 1, new_task->title, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE)
        new_task->id = sqlite3_last_insert_rowid(db);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE ? 0 : 1;
}
