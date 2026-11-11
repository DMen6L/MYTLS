#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

#include "error.h"
#include "task.h"
#include <stddef.h>
#include <string.h>

#define TASK_COLUMNS "id, title, progress, goal"
#define PAGE_SIZE 50

typedef enum {
    OP_SELECT,
    OP_INSERT,
} Operation;

// Options for operations
typedef struct {
    int limit;
    int offset;
    const char *order_by;
} SelectOpts;

typedef union {
    SelectOpts select;
} SqlOpts;

typedef struct {
    Operation op;
    SqlOpts opts;
} SqlQuery;

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

int create_conn(sqlite3 **db, const char *dbName, Error *err);
int db_ver(sqlite3 *db, Error *err);
int migrate(sqlite3 *db, Error *err);

int fetch_table(sqlite3 *db, const char *tableName, const char *const cols[],
                void *out, size_t count, Error *err);
int add_val(sqlite3 *db, const char *tableName, const char *const cols[],
            const void *const vals[], const ValueType *const types,
            size_t count, void *retrId, Error *err);

int free_table_list(void *list, const char *tableName, Error *err);

#endif
