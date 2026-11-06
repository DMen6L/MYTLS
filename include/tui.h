#ifndef TUI
#define TUI

#include "task.h"

#include <sqlite3.h>
#include <stddef.h>

#define ERROR_TEXT_LENGTH 512

typedef struct {
    TaskList tasks;
    size_t selected;
    int running;
    char err_txt[ERROR_TEXT_LENGTH];
} TuiState;

int tui_run(sqlite3 *db, TuiState *state);
void list_tasks(TaskList *tasks, int selected);
void prompt_new_task(char *title, size_t capacity, char *err_txt,
                     size_t err_capacity);
void prompt_task_del(sqlite3 *db, Task *task, char *err_txt,
                     size_t err_capacity);
void parse_controlch(sqlite3 *db, TuiState *state, int ch);

#endif // !TUI
