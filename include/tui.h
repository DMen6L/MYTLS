#ifndef TUI
#define TUI

#include "task.h"

#include <sqlite3.h>

typedef struct {
    TaskList tasks;
    size_t selected;
    int running;
} TuiState;

int tui_run(sqlite3 *db);
int list_tasks(TaskList *tasks, int selected);
int prompt_new_task(char *title, size_t capacity);
int prompt_task_del(sqlite3 *db, Task *task);
void parse_controlch(sqlite3 *db, TuiState *state, int ch);

#endif // !TUI
