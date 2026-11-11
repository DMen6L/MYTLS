#ifndef TUI
#define TUI

#include "error.h"
#include "task.h"

#include <sqlite3.h>
#include <stddef.h>

typedef struct {
    TaskList tasks;
    size_t selected;
    int running;
} TuiState;

int tui_run(sqlite3 *db, TuiState *state, Error *err);

#endif // !TUI
