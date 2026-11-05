#include "database.h"
#include "tui.h"

#include <stdio.h>
#include <string.h>

int user_manual(); // Print user manual

int cli_run(sqlite3 *db, int argc, char *argv[]);

int main(int argc, char *argv[]) {
    sqlite3 *db = NULL; // db connection

    if (create_conn(&db) != 0) {
        return 1;
    }

    if (init_database(&db) != 0) {
        return 1;
    }

    int result = cli_run(db, argc, argv);

    sqlite3_close(db);
    db = NULL;
    return result;
}

int user_manual() {
    printf("Usage: mytls <todo> <...>\n");
    return 1;
}

int cli_run(sqlite3 *db, int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "tui") == 0) {
        return tui_run(db);
    }

    return user_manual();
}
