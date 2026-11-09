#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

#include "error.h"
#include "task.h"
#include <stddef.h>

#define TASK_COLUMNS "id, title, progress, goal"
#define PAGE_SIZE 50

typedef struct {
    int version;
    const char *sql;
} Migration;

static const Migration migrations[] = {
    {
        1,
        "CREATE TABLE IF NOT EXISTS tasks ("
        "id INTEGER PRIMARY KEY,"
        "title VARCHAR(255) NOT NULL,"
        "done INTEGER NOT NULL DEFAULT 0"
        ");",
    },
    {
        2,
        "ALTER TABLE tasks ADD COLUMN goal INTEGER NOT NULL DEFAULT 1;"
        "ALTER TABLE tasks ADD COLUMN progress INTEGER NOT NULL DEFAULT 0;",
    },
    {
        3,
        "ALTER TABLE tasks DROP COLUMN done;",
    },
};

/* error must point to a valid Error. Operations clear its message on entry;
 * a nonempty message reports failure. Messages exclude trailing newlines. */
void create_conn(sqlite3 **db, Error *error);
int get_db_version(sqlite3 *db, Error *error);
void apply_migration(sqlite3 *db, Error *error);

// Tasks work
void fetch_tasks(sqlite3 *db, int page, size_t page_size, TaskList *out,
                 Error *error);
void task_list_free(TaskList *list);

/* Parallel arrays describe count fields: text values point to NUL-terminated
 * strings; integer values point to int objects. All pointers must remain valid
 * until the call returns. Fields may be omitted on insert to use SQL defaults.
 * IDs are database-owned. Both functions return 0 on success, 1 on failure.
 * add_task writes new_id only on success; new_id must point to writable storage. */
int update_task(sqlite3 *db, int task_id, const char *const *cols,
                const ValueType *types, const void *const *values,
                size_t count, Error *error);
int add_task(sqlite3 *db, const char *const *cols, const ValueType *types,
             const void *const *values, size_t count, sqlite3_int64 *new_id,
             Error *error);
void del_task(sqlite3 *db, int task_id, Error *error);

#endif
