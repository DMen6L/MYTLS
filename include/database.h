#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

int create_conn(sqlite3 **db);
int list_tasks(sqlite3 *db);
int add_task(sqlite3 *db, const char *title);

#endif
