#include "database.h"
#include "tui.h"

#include <stdio.h>
#include <string.h>

int user_manual(); // Print user manual

int cli_run(sqlite3 *db, int argc, char *argv[], TuiState *state, Error *error);

int main(int argc, char *argv[]) {
    sqlite3 *db = NULL; // db connection

    // Initialize TUI state
    TuiState state = {
        .tasks = {0},
        .selected = 0,
        .running = 1,
    };

    Error error = {0};
    create_conn(&db, &error);

    if (error.message[0] != '\0') {
        fprintf(stderr, "%s\n", error.message);
        sqlite3_close(db);
        return 1;
    }

    apply_migration(db, &error);

    if (error.message[0] != '\0') {
        fprintf(stderr, "%s\n", error.message);
        sqlite3_close(db);
        return 1;
    }

    int result = cli_run(db, argc, argv, &state, &error);

    if (error.message[0] != '\0')
        fprintf(stderr, "%s\n", error.message);

    if (sqlite3_close(db) != SQLITE_OK) {
        fprintf(stderr, "main: closing database failed: %s\n",
                sqlite3_errmsg(db));
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

int cli_run(sqlite3 *db, int argc, char *argv[], TuiState *state, Error *error) {
    if (argc == 2 && strcmp(argv[1], "tui") == 0) {
        return tui_run(db, state, error);
    }

    return user_manual();
}
