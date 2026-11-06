#include "database.h"
#include "tui.h"

#include <stdio.h>
#include <string.h>

int user_manual(); // Print user manual

int cli_run(sqlite3 *db, int argc, char *argv[], TuiState *state);

int main(int argc, char *argv[]) {
    sqlite3 *db = NULL; // db connection

    // Initialize TUI state
    TuiState state = {
        .tasks = {0},
        .selected = 0,
        .running = 1,
        .err_txt = 0,
    };

    create_conn(&db, state.err_txt, sizeof(state.err_txt));

    if (state.err_txt[0] != '\0') {
        fprintf(stderr, "%s\n", state.err_txt);
        return 1;
    }

    init_database(&db, state.err_txt, sizeof(state.err_txt));

    if (state.err_txt[0] != '\0') {
        fprintf(stderr, "%s\n", state.err_txt);
        return 1;
    }

    int result = cli_run(db, argc, argv, &state);

    if (state.err_txt[0] != '\0')
        fprintf(stderr, "%s\n", state.err_txt);

    if (sqlite3_close(db) != SQLITE_OK) {
        fprintf(stderr, "main: closing database failed: %s\n", sqlite3_errmsg(db));
        result = 1;
    } else {
        db = NULL;
    }
    return result;
}

int user_manual() {
    printf("Usage: mytls <todo> <...>\n");
    return 1;
}

int cli_run(sqlite3 *db, int argc, char *argv[], TuiState *state) {
    if (argc == 2 && strcmp(argv[1], "tui") == 0) {
        return tui_run(db, state);
    }

    return user_manual();
}
