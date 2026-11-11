#include "database.h"
#include "tui.h"

#include <stdio.h>
#include <string.h>

int user_manual(); // Print user manual
void print_err(Error *err);
int cli_run(sqlite3 *db, int argc, char *argv[], Error *err);

int main(int argc, char *argv[]) {
    sqlite3 *db = NULL; // db connection
    Error err = {0};    // Error pointer

    // Creating connection to the DB
    if (create_conn(&db, "tasks.db", &err) != 0) {
        print_err(&err);
        return 1;
    }

    // Migrating changes
    if (migrate(db, &err) != 0) {
        print_err(&err);
        return 1;
    }

    int result = cli_run(db, argc, argv, &err);

    if (result == 1)
        print_err(&err);

    return result;
}

int user_manual() {
    printf("Usage: mytls <todo> <...>\n");
    return 1;
}

void print_err(Error *err) { fprintf(stderr, "%s\n", err->message); }

int cli_run(sqlite3 *db, int argc, char *argv[], Error *err) {
    if (argc == 2 && strcmp(argv[1], "tui") == 0) {
        // Initialize TUI state
        TuiState state = {
            .tasks = {0},
            .selected = 0,
            .running = 1,
        };

        return tui_run(db, &state, err);
    }

    return user_manual();
}
