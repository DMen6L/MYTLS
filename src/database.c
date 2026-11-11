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

int create_conn(sqlite3 **db, const char *dbName, Error *err) {
    // Opening connection to the database
    int rc = sqlite3_open(dbName, db);

    // Checking for connection
    if (rc != SQLITE_OK) {
        snprintf(err->message, sizeof(err->message),
                 "create_conn: opening database %s failed: %s", dbName,
                 sqlite3_errmsg(*db));
        sqlite3_close(*db);
        *db = NULL;
        return 1;
    }
    return 0;
}

int db_ver(sqlite3 *db, Error *err) {
    if (!db) {
        snprintf(err->message, sizeof(err->message),
                 "db_ver: no db connection");
        return -1;
    }

    sqlite3_stmt *stmt = NULL;

    // Initializing the query
    int rc = sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        snprintf(err->message, sizeof(err->message),
                 "get_db_version: preparing version query failed: %s",
                 sqlite3_errmsg(db));
        return -1;
    }

    // Executing the query
    rc = sqlite3_step(stmt);
    int version = -1;

    if (rc != SQLITE_ROW) {
        snprintf(err->message, sizeof(err->message),
                 "get_db_version: reading version failed: %s",
                 sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return version;
    }

    // Reading version value
    version = sqlite3_column_int(stmt, 0);

    if (sqlite3_finalize(stmt) != SQLITE_OK) {
        snprintf(err->message, sizeof(err->message),
                 "get_db_version: finalizing version query failed: %s",
                 sqlite3_errmsg(db));
        return -1;
    }

    return version;
}

int migrate(sqlite3 *db, Error *err) {
    if (!db) {
        snprintf(err->message, sizeof(err->message),
                 "apply_migration: database connection is required");
        return 1;
    }

    int current = db_ver(db, err);
    if (current < 0)
        return 1;

    size_t count = sizeof(migrations) / sizeof(migrations[0]);
    int latest = count > 0 ? migrations[count - 1].version : 0;

    if (current > latest) {
        snprintf(err->message, sizeof(err->message),
                 "apply_migration: database version %d is newer than supported "
                 "version %d",
                 current, latest);
        return 1;
    }

    for (size_t i = 0; i < count; i++) {
        const Migration *m = &migrations[i];
        if (m->version <= current)
            continue;

        // Initializing the transaction
        if (sqlite3_exec(db, "BEGIN IMMEDIATE;", NULL, NULL, NULL) !=
            SQLITE_OK) {
            snprintf(err->message, sizeof(err->message),
                     "apply_migration: version %d begin failed: %s", m->version,
                     sqlite3_errmsg(db));
            return 1;
        }

        /* Another process may have migrated while we waited for the lock. */
        current = db_ver(db, err);
        if (current < 0)
            goto rollback;
        if (current > latest) {
            snprintf(err->message, sizeof(err->message),
                     "apply_migration: database version %d is newer than "
                     "supported version %d",
                     current, latest);
            goto rollback;
        }

        // Executing migration
        const char *stage = "SQL execution";
        int rc = SQLITE_OK;

        if (m->version > current) {
            // Migrating changes
            rc = sqlite3_exec(db, m->sql, NULL, NULL, NULL);
            if (rc == SQLITE_OK) {
                char sql[64];
                snprintf(sql, sizeof(sql), "PRAGMA user_version = %d;",
                         m->version);
                stage = "version update";
                rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
            }
        }

        // Commiting changes
        if (rc == SQLITE_OK) {
            stage = "commit";
            rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
        }
        // Updating current migrated version
        if (rc == SQLITE_OK) {
            if (m->version > current)
                current = m->version;
            continue;
        }

        snprintf(err->message, sizeof(err->message),
                 "apply_migration: version %d %s failed: %s", m->version, stage,
                 sqlite3_errmsg(db));
    rollback:
        /* Some SQLite failures automatically roll back the transaction. */
        if (!sqlite3_get_autocommit(db) &&
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL) != SQLITE_OK) {
            size_t used = strlen(err->message);
            snprintf(err->message + used, sizeof(err->message) - used,
                     "; rollback failed: %s", sqlite3_errmsg(db));
        }
        return 1;
    }

    return 0;
}

// General SQL value reader
int row2val(sqlite3_stmt *stmt, const char *tableName, void *dest, Error *err) {
    if (strcmp(tableName, "tasks") == 0) {
        Task *task = (Task *)dest;

        task->id = sqlite3_column_int(stmt, 0);
        unsigned const char *title = sqlite3_column_text(stmt, 1);

        if (!title) {
            snprintf(err->message, sizeof(err->message),
                     "row2val: NULL pointer title");
            return 1;
        }

        snprintf(task->title, sizeof(task->title), "%s", (const char *)title);

        task->progress = sqlite3_column_int(stmt, 2);
        task->goal = sqlite3_column_int(stmt, 3);

        return 0;
    }

    snprintf(err->message, sizeof(err->message),
             "row2val: unsupported table %s", tableName);
    return 1;
}

// General SQL statement builder
void sql_build(sqlite3_str *builder, SqlQuery query, const char *tableName,
               const char *const cols[], size_t count, Error *err) {
    switch (query.op) {
    case OP_SELECT:
        sqlite3_str_appendall(builder, "SELECT ");

        for (size_t i = 0; i < count; i++) {
            if (i > 0) {
                sqlite3_str_appendall(builder, ", ");
            }

            sqlite3_str_appendf(builder, "\"%w\"", cols[i]);
        }

        sqlite3_str_appendf(builder, " FROM \"%w\"", tableName);

        SelectOpts *opts = &query.opts.select;

        // Apply options
        if (opts->order_by)
            sqlite3_str_appendf(builder, " ORDER BY %s", opts->order_by);
        if (opts->limit > 0)
            sqlite3_str_appendf(builder, " LIMIT %d", opts->limit);
        if (opts->offset > 0)
            sqlite3_str_appendf(builder, " OFFSET %d", opts->offset);

        sqlite3_str_appendall(builder, ";");
        break;

    case OP_INSERT:
        sqlite3_str_appendf(builder, "INSERT INTO \"%w\" (", tableName);

        for (size_t i = 0; i < count; i++) {
            if (i > 0) {
                sqlite3_str_appendall(builder, ", ");
            }

            sqlite3_str_appendf(builder, "\"%w\"", cols[i]);
        }

        sqlite3_str_appendall(builder, ") VALUES (");

        for (size_t i = 0; i < count; i++) {
            if (i > 0) {
                sqlite3_str_appendall(builder, ", ");
            }

            sqlite3_str_appendall(builder, "?");
        }

        sqlite3_str_appendall(builder, ");");
        break;

    default:
        break;
    }
}

int fetch_table(sqlite3 *db, const char *tableName, const char *const cols[],
                void *out, size_t count, Error *err) {
    if (!db) {
        snprintf(err->message, sizeof(err->message),
                 "fetch_table: No db Connection");
        return 1;
    }

    if (!tableName || !cols || !out) {
        snprintf(err->message, sizeof(err->message),
                 "fetch_table: Incorrectly passed arguments");
        return 1;
    }

    sqlite3_str *builder = sqlite3_str_new(db);
    if (!builder) {
        snprintf(err->message, sizeof(err->message),
                 "fetch_table: builder allocation failed");
        return 1;
    }

    sql_build(builder,
              (SqlQuery){
                  .op = OP_SELECT,
                  .opts.select = {.limit = PAGE_SIZE,
                                  .offset = 0,
                                  .order_by = "progress, id"},
              },
              tableName, cols, count, err);

    char *sql = sqlite3_str_finish(builder);

    if (!sql) {
        snprintf(err->message, sizeof(err->message),
                 "fetch_table: SQL build failed");
        return 1;
    }

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_free(sql);

    if (rc != SQLITE_OK)
        goto fail;

    if (strcmp(tableName, "tasks") == 0) {
        TaskList *tasks = out;

        tasks->count = 0;
        tasks->items = malloc(PAGE_SIZE * sizeof(Task));

        if (!tasks->items) {
            snprintf(err->message, sizeof(err->message),
                     "fetch_table: memory allocation failed");
            sqlite3_finalize(stmt);
            return 1;
        }

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            if (tasks->count >= PAGE_SIZE)
                break;

            Task *task = &tasks->items[tasks->count];

            if (row2val(stmt, tableName, task, err) != 0) {
                sqlite3_finalize(stmt);
                return 1;
            }

            tasks->count++;
        }
    } else {
        snprintf(err->message, sizeof(err->message),
                 "fetch_table: unsupported table %s", tableName);
        sqlite3_finalize(stmt);
        return 1;
    }

    if (rc != SQLITE_DONE)
        goto fail;

    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        snprintf(err->message, sizeof(err->message),
                 "fetch_table: finalize failed %s", sqlite3_errmsg(db));
        return 1;
    }

    return 0;

fail:
    snprintf(err->message, sizeof(err->message), "fetch_table: %s",
             sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return 1;
}

int bind_val(sqlite3_stmt *stmt, int index, const void *val, ValueType type) {
    if (!stmt || !val)
        return SQLITE_MISUSE;

    switch (type) {
    case VALUE_INT:
        return sqlite3_bind_int(stmt, index, *(const int *)val);
        break;
    case VALUE_TEXT:
        return sqlite3_bind_text(stmt, index, (const char *)val, -1,
                                 SQLITE_TRANSIENT);
    default:
        return SQLITE_MISUSE;
    }
}

int add_val(sqlite3 *db, const char *tableName, const char *const cols[],
            const void *const vals[], const ValueType *const types,
            size_t count, void *retrId, Error *err) {
    if (!db || !cols || !vals) {
        snprintf(err->message, sizeof(err->message),
                 "add_val: Incorrect passed arguments");
        return 1;
    }

    sqlite3_str *builder = sqlite3_str_new(db);

    sql_build(builder, (SqlQuery){.op = OP_INSERT, .opts = {0}}, tableName,
              cols, count, err);

    char *sql = sqlite3_str_finish(builder);

    if (!sql) {
        snprintf(err->message, sizeof(err->message),
                 "fetch_table: SQL build failed");
        return 1;
    }

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_free(sql);

    if (rc != SQLITE_OK)
        goto fail;

    for (size_t i = 0; i < count; i++) {
        rc = bind_val(stmt, (int)i + 1, vals[i], types[i]);

        if (rc != SQLITE_OK)
            goto fail;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE)
        goto fail;

    rc = sqlite3_finalize(stmt);

    if (rc != SQLITE_OK) {
        snprintf(err->message, sizeof(err->message),
                 "add_val: finalize failed: %s", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_int64 id = sqlite3_last_insert_rowid(db);
    if (id < INT_MIN || id > INT_MAX) {
        snprintf(err->message, sizeof(err->message),
                 "add_val: Task ID exceeds supported range");
        return 1;
    }
    if (strcmp(tableName, "tasks") == 0) {
        int *taskId = retrId;
        *taskId = (int)id;
    }

    return 0;

fail:
    snprintf(err->message, sizeof(err->message), "add_val: %s",
             sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return 1;
}

int free_table_list(void *list, const char *tableName, Error *err) {
    if (!list || !tableName) {
        snprintf(err->message, sizeof(err->message),
                 "free_table_list: invalid arguments");
        return 1;
    }

    if (strcmp(tableName, "tasks") == 0) {
        TaskList *tasks = list;

        free(tasks->items);
        tasks->items = NULL;
        tasks->count = 0;

        return 0;
    }

    snprintf(err->message, sizeof(err->message),
             "free_table_list: incorrect table name %s", tableName);
    return 1;
}
