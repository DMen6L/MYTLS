#include "database.h"
#include "task.h"

#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void create_conn(sqlite3 **db, char *err_txt, size_t err_capacity) {
    if (err_capacity > 0) {
        err_txt[0] = '\0';
    }

    int rc = sqlite3_open("tasks.db", db); // Opening the db

    // Checking for connection
    if (rc != SQLITE_OK) {
        snprintf(err_txt, err_capacity,
                 "create_conn: opening database failed: %s",
                 sqlite3_errmsg(*db));
        sqlite3_close(*db);
        *db = NULL;
    }
}

int get_db_version(sqlite3 *db, char *err_txt, size_t err_capacity) {
    if (err_capacity > 0) {
        err_txt[0] = '\0';
    }

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        snprintf(err_txt, err_capacity,
                 "get_db_version: preparing version query failed: %s",
                 sqlite3_errmsg(db));
        return -1;
    }

    rc = sqlite3_step(stmt);
    int version = -1;

    if (rc != SQLITE_ROW) {
        snprintf(err_txt, err_capacity,
                 "get_db_version: reading version failed: %s",
                 sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return version;
    }

    version = sqlite3_column_int(stmt, 0);
    if (sqlite3_finalize(stmt) != SQLITE_OK) {
        snprintf(err_txt, err_capacity,
                 "get_db_version: finalizing version query failed: %s",
                 sqlite3_errmsg(db));
        return -1;
    }
    return version;
}

void apply_migration(sqlite3 *db, char *err_txt, size_t err_capacity) {
    if (err_capacity > 0)
        err_txt[0] = '\0';
    if (db == NULL) {
        snprintf(err_txt, err_capacity,
                 "apply_migration: database connection is required");
        return;
    }

    int current = get_db_version(db, err_txt, err_capacity);
    if (current < 0)
        return; /* Preserve the detailed version-reading error. */

    size_t count = sizeof(migrations) / sizeof(migrations[0]);
    int latest = count > 0 ? migrations[count - 1].version : 0;
    if (current > latest) {
        snprintf(err_txt, err_capacity,
                 "apply_migration: database version %d is newer than supported version %d",
                 current, latest);
        return;
    }

    for (size_t i = 0; i < count; i++) {
        const Migration *m = &migrations[i];
        if (m->version <= current)
            continue;

        if (sqlite3_exec(db, "BEGIN IMMEDIATE;", NULL, NULL, NULL) != SQLITE_OK) {
            snprintf(err_txt, err_capacity,
                     "apply_migration: version %d begin failed: %s",
                     m->version, sqlite3_errmsg(db));
            return;
        }

        /* Another process may have migrated while we waited for the lock. */
        current = get_db_version(db, err_txt, err_capacity);
        if (current < 0)
            goto rollback;
        if (current > latest) {
            snprintf(err_txt, err_capacity,
                     "apply_migration: database version %d is newer than supported version %d",
                     current, latest);
            goto rollback;
        }

        const char *stage = "SQL execution";
        int rc = SQLITE_OK;
        if (m->version > current) {
            rc = sqlite3_exec(db, m->sql, NULL, NULL, NULL);
            if (rc == SQLITE_OK) {
                char sql[64];
                snprintf(sql, sizeof(sql), "PRAGMA user_version = %d;", m->version);
                stage = "version update";
                rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
            }
        }
        if (rc == SQLITE_OK) {
            stage = "commit";
            rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
        }
        if (rc == SQLITE_OK) {
            if (m->version > current)
                current = m->version;
            continue;
        }

        snprintf(err_txt, err_capacity,
                 "apply_migration: version %d %s failed: %s",
                 m->version, stage, sqlite3_errmsg(db));

rollback:
        /* Some SQLite failures automatically roll back the transaction. */
        if (!sqlite3_get_autocommit(db) &&
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL) != SQLITE_OK &&
            err_capacity > 0) {
            size_t used = strlen(err_txt);
            snprintf(err_txt + used, err_capacity - used,
                     "; rollback failed: %s", sqlite3_errmsg(db));
        }
        return;
    }
}

void fetch_tasks(sqlite3 *db, int page, size_t page_size, TaskList *out,
                 char *err_txt, size_t err_capacity) {
    if (err_capacity > 0) {
        err_txt[0] = '\0';
    }

    if (db == NULL || out == NULL || out->items != NULL || out->count != 0) {
        snprintf(err_txt, err_capacity,
                 "fetch_tasks: database and output are required; output must "
                 "be empty");
        return;
    }

    if (page < 0 || page_size == 0 || page_size > SIZE_MAX / sizeof(Task) ||
        (uintmax_t)page_size > INT64_MAX) {
        snprintf(err_txt, err_capacity,
                 "fetch_tasks: invalid page number or page size");
        return;
    }

    sqlite3_int64 limit = (sqlite3_int64)page_size;

    if (page != 0 && limit > INT64_MAX / page) {
        snprintf(err_txt, err_capacity,
                 "fetch_tasks: page offset exceeds SQLite integer range");
        return;
    }

    sqlite3_int64 offset = (sqlite3_int64)page * limit;
    sqlite3_stmt *stmt = NULL;
    Task *items = NULL;
    size_t count = 0;

    const char *sql = "SELECT id, title, done FROM tasks "
                      "ORDER BY id LIMIT ? OFFSET ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        snprintf(err_txt, err_capacity,
                 "fetch_tasks: preparing task query failed: %s",
                 sqlite3_errmsg(db));
        goto fail;
    }

    if (sqlite3_bind_int64(stmt, 1, limit) != SQLITE_OK ||
        sqlite3_bind_int64(stmt, 2, offset) != SQLITE_OK) {
        snprintf(err_txt, err_capacity,
                 "fetch_tasks: binding pagination parameters failed: %s",
                 sqlite3_errmsg(db));
        goto fail;
    }

    items = malloc(page_size * sizeof(*items));
    if (items == NULL) {
        snprintf(err_txt, err_capacity,
                 "fetch_tasks: allocating task buffer failed");
        goto fail;
    }

    int rc;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (count >= page_size) {
            snprintf(err_txt, err_capacity,
                     "fetch_tasks: query returned more tasks than allocated "
                     "capacity");
            goto fail;
        }

        Task *task = &items[count];
        task->id = sqlite3_column_int(stmt, 0);

        const unsigned char *title = sqlite3_column_text(stmt, 1);
        if (title == NULL) {
            snprintf(err_txt, err_capacity,
                     "fetch_tasks: title is NULL or could not be read");
            goto fail;
        }

        snprintf(task->title, sizeof(task->title), "%s", (const char *)title);

        task->done = sqlite3_column_int(stmt, 2);
        count++;
    }

    if (rc != SQLITE_DONE) {
        snprintf(err_txt, err_capacity,
                 "fetch_tasks: reading task rows failed: %s",
                 sqlite3_errmsg(db));
        goto fail;
    }

    int finalize_rc = sqlite3_finalize(stmt);
    stmt = NULL;

    if (finalize_rc != SQLITE_OK) {
        snprintf(err_txt, err_capacity,
                 "fetch_tasks: finalizing task query failed: %s",
                 sqlite3_errmsg(db));
        goto fail;
    }

    if (count == 0) {
        free(items);
        items = NULL;
    }

    out->items = items;
    out->count = count;
    return;

fail:
    sqlite3_finalize(stmt);
    free(items);
    return;
}

void task_list_free(TaskList *list) {
    if (list == NULL) {
        return;
    }

    free(list->items);
    list->items = NULL;
    list->count = 0;
}

/* Preserve execution errors before finalizing the statement. */
static int execute_change(sqlite3 *db, sqlite3_stmt *stmt,
                          const char *operation, int require_match,
                          char *err_txt, size_t err_capacity) {
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        snprintf(err_txt, err_capacity, "%s: executing statement failed: %s",
                 operation, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }

    int changed = sqlite3_changes(db);
    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        snprintf(err_txt, err_capacity, "%s: finalizing statement failed: %s",
                 operation, sqlite3_errmsg(db));
        return 1;
    }
    if (require_match && changed == 0) {
        snprintf(err_txt, err_capacity, "%s: task not found", operation);
        return 1;
    }
    return 0;
}

void set_task_done(sqlite3 *db, int task_id, int done, char *err_txt,
                   size_t err_capacity) {
    if (err_capacity > 0)
        err_txt[0] = '\0';
    if (db == NULL || task_id <= 0 || (done != 0 && done != 1)) {
        snprintf(err_txt, err_capacity,
                 "set_task_done: database, positive task ID, and boolean "
                 "status required");
        return;
    }

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, "UPDATE tasks SET done = ? WHERE id = ?;", -1,
                           &stmt, NULL) != SQLITE_OK) {
        snprintf(err_txt, err_capacity,
                 "set_task_done: preparing statement failed: %s",
                 sqlite3_errmsg(db));
        return;
    }
    if (sqlite3_bind_int(stmt, 1, done) != SQLITE_OK ||
        sqlite3_bind_int(stmt, 2, task_id) != SQLITE_OK) {
        snprintf(err_txt, err_capacity,
                 "set_task_done: binding parameters failed: %s",
                 sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }
    execute_change(db, stmt, "set_task_done", 1, err_txt, err_capacity);
}

void add_task(sqlite3 *db, Task *new_task, char *err_txt, size_t err_capacity) {
    if (err_capacity > 0)
        err_txt[0] = '\0';
    if (db == NULL || new_task == NULL) {
        snprintf(err_txt, err_capacity,
                 "add_task: database and task are required");
        return;
    }
    const char *end = memchr(new_task->title, '\0', sizeof(new_task->title));
    if (end == NULL) {
        snprintf(err_txt, err_capacity,
                 "add_task: title exceeds maximum length");
        return;
    }
    const char *text = new_task->title;
    while (text < end && isspace((unsigned char)*text))
        text++;
    if (text == end) {
        snprintf(err_txt, err_capacity, "add_task: title must not be blank");
        return;
    }

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, "INSERT INTO tasks (title) VALUES(?);", -1,
                           &stmt, NULL) != SQLITE_OK) {
        snprintf(err_txt, err_capacity,
                 "add_task: preparing statement failed: %s",
                 sqlite3_errmsg(db));
        return;
    }
    if (sqlite3_bind_text(stmt, 1, new_task->title, -1, SQLITE_TRANSIENT) !=
        SQLITE_OK) {
        snprintf(err_txt, err_capacity, "add_task: binding title failed: %s",
                 sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }
    if (execute_change(db, stmt, "add_task", 0, err_txt, err_capacity) != 0)
        return;

    sqlite3_int64 id = sqlite3_last_insert_rowid(db);
    if (id <= 0 || id > INT_MAX) {
        snprintf(err_txt, err_capacity,
                 "add_task: task was inserted but its ID exceeds the supported "
                 "range");
        return;
    }
    new_task->id = (int)id;
}

void del_task(sqlite3 *db, int task_id, char *err_txt, size_t err_capacity) {
    if (err_capacity > 0)
        err_txt[0] = '\0';
    if (db == NULL || task_id <= 0) {
        snprintf(err_txt, err_capacity,
                 "del_task: database and positive task ID required");
        return;
    }

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, "DELETE FROM tasks WHERE id = ?;", -1, &stmt,
                           NULL) != SQLITE_OK) {
        snprintf(err_txt, err_capacity,
                 "del_task: preparing statement failed: %s",
                 sqlite3_errmsg(db));
        return;
    }
    if (sqlite3_bind_int(stmt, 1, task_id) != SQLITE_OK) {
        snprintf(err_txt, err_capacity, "del_task: binding task ID failed: %s",
                 sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }
    execute_change(db, stmt, "del_task", 1, err_txt, err_capacity);
}
