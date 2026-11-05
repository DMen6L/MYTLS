#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

#include "task.h"
#include <stddef.h>

int create_conn(sqlite3 **db);
int init_database(sqlite3 **db);

// Tasks work
int fetch_tasks(sqlite3 *db, int page, size_t page_size, TaskList *out);
void task_list_free(TaskList *list);

int set_task_done(sqlite3 *db, int task_id, int done);
int add_task(sqlite3 *db, Task *new_task);
int del_task(sqlite3 *db, int task_id);

#endif
