#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

#include "task.h"
#include <stddef.h>

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
};

/* Error buffers are cleared on entry. For positive err_capacity, err_txt must
 * point to at least err_capacity writable bytes. Messages exclude newlines.
 * Pass NULL with capacity 0 only when error text is not needed. */
void create_conn(sqlite3 **db, char *err_txt, size_t err_capacity);
int get_db_version(sqlite3 *db, char *err_txt, size_t err_capacity);
void init_database(sqlite3 **db, char *err_txt, size_t err_capacity);

// Tasks work
void fetch_tasks(sqlite3 *db, int page, size_t page_size, TaskList *out,
                 char *err_txt, size_t err_capacity);
void task_list_free(TaskList *list);

void set_task_done(sqlite3 *db, int task_id, int done, char *err_txt,
                   size_t err_capacity);
void add_task(sqlite3 *db, Task *new_task, char *err_txt, size_t err_capacity);
void del_task(sqlite3 *db, int task_id, char *err_txt, size_t err_capacity);

#endif
