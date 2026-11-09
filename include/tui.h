#ifndef TUI
#define TUI

#include "task.h"
#include "error.h"

#include <sqlite3.h>
#include <stddef.h>

typedef struct {
    TaskList tasks;
    size_t selected;
    int running;
} TuiState;

int tui_run(sqlite3 *db, TuiState *state, Error *error);
void list_tasks(const TaskList *tasks, size_t selected);
void prompt_new_task(char *title, size_t capacity, Error *error);
/* Return 1 for confirmation, 0 for cancellation, -1 for failure. */
int prompt_task_del(const Task *task, Error *error);
void parse_controlch(sqlite3 *db, TuiState *state, int ch, Error *error);

#endif // !TUI
