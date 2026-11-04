#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

#include "task.h"
#include <stddef.h>

typedef struct {
    Task *items;
    size_t count;
} TaskList;

int create_conn(sqlite3 **db);

// Tasks work
int fetch_tasks(sqlite3 *db, int page, size_t page_size, TaskList *out);
void task_list_free(TaskList *list);

int set_task_done(sqlite3 *db, int task_id, int done);
int add_task(sqlite3 *db, Task *new_task);

#endif
