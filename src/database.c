#include "database.h"
#include "error.h"
#include "task.h"

#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void create_conn(sqlite3 **db, Error *error) {
    error->message[0] = '\0';

    // Opening connection to the database
    int rc = sqlite3_open("tasks.db", db);

    // Checking for connection
    if (rc != SQLITE_OK) {
        snprintf(error->message, sizeof(error->message),
                 "create_conn: opening database failed: %s",
                 sqlite3_errmsg(*db));
        sqlite3_close(*db);
        *db = NULL;
    }
}

int get_db_version(sqlite3 *db, Error *error) {
    error->message[0] = '\0';

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        snprintf(error->message, sizeof(error->message),
                 "get_db_version: preparing version query failed: %s",
                 sqlite3_errmsg(db));
        return -1;
    }

    rc = sqlite3_step(stmt);
    int version = -1;

    if (rc != SQLITE_ROW) {
        snprintf(error->message, sizeof(error->message),
                 "get_db_version: reading version failed: %s",
                 sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return version;
    }

    version = sqlite3_column_int(stmt, 0);
    if (sqlite3_finalize(stmt) != SQLITE_OK) {
        snprintf(error->message, sizeof(error->message),
                 "get_db_version: finalizing version query failed: %s",
                 sqlite3_errmsg(db));
        return -1;
    }
    return version;
}

void apply_migration(sqlite3 *db, Error *error) {
    error->message[0] = '\0';

    if (db == NULL) {
        snprintf(error->message, sizeof(error->message),
                 "apply_migration: database connection is required");
        return;
    }

    int current = get_db_version(db, error);
    if (current < 0)
        return; /* Preserve the detailed version-reading error. */

    size_t count = sizeof(migrations) / sizeof(migrations[0]);
    int latest = count > 0 ? migrations[count - 1].version : 0;
    if (current > latest) {
        snprintf(error->message, sizeof(error->message),
                 "apply_migration: database version %d is newer than supported "
                 "version %d",
                 current, latest);
        return;
    }

    for (size_t i = 0; i < count; i++) {
        const Migration *m = &migrations[i];
        if (m->version <= current)
            continue;

        if (sqlite3_exec(db, "BEGIN IMMEDIATE;", NULL, NULL, NULL) !=
            SQLITE_OK) {
            snprintf(error->message, sizeof(error->message),
                     "apply_migration: version %d begin failed: %s", m->version,
                     sqlite3_errmsg(db));
            return;
        }

        /* Another process may have migrated while we waited for the lock. */
        current = get_db_version(db, error);
        if (current < 0)
            goto rollback;
        if (current > latest) {
            snprintf(error->message, sizeof(error->message),
                     "apply_migration: database version %d is newer than "
                     "supported version %d",
                     current, latest);
            goto rollback;
        }

        const char *stage = "SQL execution";
        int rc = SQLITE_OK;
        if (m->version > current) {
            rc = sqlite3_exec(db, m->sql, NULL, NULL, NULL);
            if (rc == SQLITE_OK) {
                char sql[64];
                snprintf(sql, sizeof(sql), "PRAGMA user_version = %d;",
                         m->version);
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

        snprintf(error->message, sizeof(error->message),
                 "apply_migration: version %d %s failed: %s", m->version, stage,
                 sqlite3_errmsg(db));

    rollback:
        /* Some SQLite failures automatically roll back the transaction. */
        if (!sqlite3_get_autocommit(db) &&
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL) != SQLITE_OK) {
            size_t used = strlen(error->message);
            snprintf(error->message + used, sizeof(error->message) - used,
                     "; rollback failed: %s", sqlite3_errmsg(db));
        }
        return;
    }
}

void task_from_row(sqlite3_stmt *stmt, Task *task, Error *error) {
    task->id = sqlite3_column_int(stmt, 0);

    const unsigned char *title = sqlite3_column_text(stmt, 1);
    if (title == NULL) {
        snprintf(error->message, sizeof(error->message),
                 "fetch_tasks: title is NULL or could not be read");
        return;
    }

    snprintf(task->title, sizeof(task->title), "%s", (const char *)title);

    task->progress = sqlite3_column_int(stmt, 2);
    task->goal = sqlite3_column_int(stmt, 3);
}

void fetch_tasks(sqlite3 *db, int page, size_t page_size, TaskList *out,
                 Error *error) {
    error->message[0] = '\0';

    if (db == NULL || out == NULL || out->items != NULL || out->count != 0) {
        snprintf(error->message, sizeof(error->message),
                 "fetch_tasks: database and output are required; output must "
                 "be empty");
        return;
    }

    if (page < 0 || page_size == 0 || page_size > SIZE_MAX / sizeof(Task) ||
        (uintmax_t)page_size > INT64_MAX) {
        snprintf(error->message, sizeof(error->message),
                 "fetch_tasks: invalid page number or page size");
        return;
    }

    sqlite3_int64 limit = (sqlite3_int64)page_size;

    if (page != 0 && limit > INT64_MAX / page) {
        snprintf(error->message, sizeof(error->message),
                 "fetch_tasks: page offset exceeds SQLite integer range");
        return;
    }

    sqlite3_int64 offset = (sqlite3_int64)page * limit;
    sqlite3_stmt *stmt = NULL;
    Task *items = NULL;
    size_t count = 0;

    const char *sql = "SELECT " TASK_COLUMNS " FROM tasks "
                      "ORDER BY progress, id LIMIT ? OFFSET ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        snprintf(error->message, sizeof(error->message),
                 "fetch_tasks: preparing task query failed: %s",
                 sqlite3_errmsg(db));
        goto fail;
    }

    if (sqlite3_bind_int64(stmt, 1, limit) != SQLITE_OK ||
        sqlite3_bind_int64(stmt, 2, offset) != SQLITE_OK) {
        snprintf(error->message, sizeof(error->message),
                 "fetch_tasks: binding pagination parameters failed: %s",
                 sqlite3_errmsg(db));
        goto fail;
    }

    items = malloc(page_size * sizeof(*items));
    if (items == NULL) {
        snprintf(error->message, sizeof(error->message),
                 "fetch_tasks: allocating task buffer failed");
        goto fail;
    }

    int rc;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (count >= page_size) {
            snprintf(error->message, sizeof(error->message),
                     "fetch_tasks: query returned more tasks than allocated "
                     "capacity");
            goto fail;
        }

        Task *task = &items[count];
        task_from_row(stmt, task, error);
        if (error->message[0] != '\0')
            goto fail;

        count++;
    }

    if (rc != SQLITE_DONE) {
        snprintf(error->message, sizeof(error->message),
                 "fetch_tasks: reading task rows failed: %s",
                 sqlite3_errmsg(db));
        goto fail;
    }

    int finalize_rc = sqlite3_finalize(stmt);
    stmt = NULL;

    if (finalize_rc != SQLITE_OK) {
        snprintf(error->message, sizeof(error->message),
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
                          Error *error) {
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        snprintf(error->message, sizeof(error->message),
                 "%s: executing statement failed: %s", operation,
                 sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }

    int changed = sqlite3_changes(db);
    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        snprintf(error->message, sizeof(error->message),
                 "%s: finalizing statement failed: %s", operation,
                 sqlite3_errmsg(db));
        return 1;
    }
    if (require_match && changed == 0) {
        snprintf(error->message, sizeof(error->message), "%s: task not found",
                 operation);
        return 1;
    }
    return 0;
}

/* The same validation, SQL construction, and bindings serve inserts and
 * updates. A zero task_id selects insertion; public update_task requires a
 * positive ID. */
static int write_task(sqlite3 *db, int task_id, const char *const *cols,
                      const ValueType *types, const void *const *values,
                      size_t count, Error *error) {
    const char *operation = task_id == 0 ? "add_task" : "update_task";
    error->message[0] = '\0';
    if (db == NULL || cols == NULL || types == NULL || values == NULL ||
        count == 0) {
        snprintf(
            error->message, sizeof(error->message),
            "%s: database and nonempty field, type, and value arrays required",
            operation);
        return 1;
    }
    int extra = task_id != 0;
    if (count > (size_t)sqlite3_limit(db, SQLITE_LIMIT_COLUMN, -1) ||
        count > (size_t)(sqlite3_limit(db, SQLITE_LIMIT_VARIABLE_NUMBER, -1) -
                         extra)) {
        snprintf(error->message, sizeof(error->message),
                 "%s: field count exceeds SQLite limits", operation);
        return 1;
    }

    for (size_t i = 0; i < count; i++) {
        if (cols[i] == NULL || cols[i][0] == '\0' || values[i] == NULL) {
            snprintf(error->message, sizeof(error->message),
                     "%s: each field requires a name and value", operation);
            return 1;
        }
        if (sqlite3_stricmp(cols[i], "id") == 0 ||
            sqlite3_stricmp(cols[i], "rowid") == 0 ||
            sqlite3_stricmp(cols[i], "_rowid_") == 0 ||
            sqlite3_stricmp(cols[i], "oid") == 0) {
            snprintf(error->message, sizeof(error->message),
                     "%s: task IDs are managed by the database", operation);
            return 1;
        }
        for (size_t j = 0; j < i; j++) {
            if (sqlite3_stricmp(cols[i], cols[j]) == 0) {
                snprintf(error->message, sizeof(error->message),
                         "%s: duplicate field: %s", operation, cols[i]);
                return 1;
            }
        }
        if (types[i] != VALUE_INT && types[i] != VALUE_TEXT) {
            snprintf(error->message, sizeof(error->message),
                     "%s: unsupported value type for field: %s", operation,
                     cols[i]);
            return 1;
        }
        if (sqlite3_stricmp(cols[i], "title") == 0) {
            if (types[i] != VALUE_TEXT) {
                snprintf(error->message, sizeof(error->message),
                         "%s: title requires a text value", operation);
                return 1;
            }
            const char *text = values[i];
            size_t length = strlen(text);
            while (isspace((unsigned char)*text))
                text++;
            if (length >= TITLE_SIZE || *text == '\0') {
                snprintf(error->message, sizeof(error->message),
                         "%s: title must be nonblank and shorter than %d bytes",
                         operation, TITLE_SIZE);
                return 1;
            }
        } else if (sqlite3_stricmp(cols[i], "goal") == 0 ||
                   sqlite3_stricmp(cols[i], "progress") == 0) {
            if (types[i] != VALUE_INT) {
                snprintf(error->message, sizeof(error->message),
                         "%s: %s requires an integer value", operation,
                         cols[i]);
                return 1;
            }
            int value = *(const int *)values[i];
            if (value < 0 ||
                (value == 0 && sqlite3_stricmp(cols[i], "goal") == 0)) {
                snprintf(error->message, sizeof(error->message),
                         "%s: goal must be positive and progress nonnegative",
                         operation);
                return 1;
            }
        }
    }

    sqlite3_str *builder = sqlite3_str_new(db);
    sqlite3_str_appendall(builder, task_id == 0 ? "INSERT INTO tasks ("
                                                : "UPDATE tasks SET ");
    for (size_t i = 0; i < count; i++) {
        /* Identifiers are quoted; values are always bound separately. */
        sqlite3_str_appendf(builder, "%s\"%w\"%s", i == 0 ? "" : ", ", cols[i],
                            task_id == 0 ? "" : " = ?");
    }
    if (task_id == 0) {
        sqlite3_str_appendall(builder, ") VALUES (");
        for (size_t i = 0; i < count; i++)
            sqlite3_str_appendall(builder, i == 0 ? "?" : ", ?");
        sqlite3_str_appendall(builder, ");");
    } else {
        sqlite3_str_appendall(builder, " WHERE id = ?;");
    }
    char *sql = sqlite3_str_finish(builder);
    if (sql == NULL) {
        snprintf(error->message, sizeof(error->message),
                 "%s: building SQL failed", operation);
        return 1;
    }

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_free(sql);
    if (rc != SQLITE_OK) {
        snprintf(error->message, sizeof(error->message),
                 "%s: preparing statement failed: %s", operation,
                 sqlite3_errmsg(db));
        return 1;
    }
    for (size_t i = 0; i < count; i++) {
        int index = (int)i + 1;
        rc = types[i] == VALUE_INT
                 ? sqlite3_bind_int(stmt, index, *(const int *)values[i])
                 : sqlite3_bind_text(stmt, index, values[i], -1,
                                     SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
            goto bind_failed;
    }
    if (task_id != 0 &&
        sqlite3_bind_int(stmt, (int)count + 1, task_id) != SQLITE_OK)
        goto bind_failed;

    return execute_change(db, stmt, operation, task_id != 0, error);

bind_failed:
    snprintf(error->message, sizeof(error->message),
             "%s: binding parameters failed: %s", operation,
             sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return 1;
}

int update_task(sqlite3 *db, int task_id, const char *const *cols,
                const ValueType *types, const void *const *values, size_t count,
                Error *error) {
    if (task_id <= 0) {
        snprintf(error->message, sizeof(error->message),
                 "update_task: positive task ID required");
        return 1;
    }
    return write_task(db, task_id, cols, types, values, count, error);
}

int add_task(sqlite3 *db, const char *const *cols, const ValueType *types,
             const void *const *values, size_t count, sqlite3_int64 *new_id,
             Error *error) {
    if (new_id == NULL) {
        snprintf(error->message, sizeof(error->message),
                 "add_task: output ID is required");
        return 1;
    }
    if (write_task(db, 0, cols, types, values, count, error) != 0)
        return 1;
    if (sqlite3_changes(db) == 0) {
        snprintf(error->message, sizeof(error->message),
                 "add_task: statement completed without inserting a task");
        return 1;
    }
    *new_id = sqlite3_last_insert_rowid(db);
    return 0;
}

void del_task(sqlite3 *db, int task_id, Error *error) {
    error->message[0] = '\0';
    if (db == NULL || task_id <= 0) {
        snprintf(error->message, sizeof(error->message),
                 "del_task: database and positive task ID required");
        return;
    }

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, "DELETE FROM tasks WHERE id = ?;", -1, &stmt,
                           NULL) != SQLITE_OK) {
        snprintf(error->message, sizeof(error->message),
                 "del_task: preparing statement failed: %s",
                 sqlite3_errmsg(db));
        return;
    }
    if (sqlite3_bind_int(stmt, 1, task_id) != SQLITE_OK) {
        snprintf(error->message, sizeof(error->message),
                 "del_task: binding task ID failed: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }
    execute_change(db, stmt, "del_task", 1, error);
}
