#include "database.h"

#include <stdio.h>
#include <string.h>

#define TITLE_SIZE 256

int user_manual(); // Print user manual

int main(int argc, char *argv[]) {
    if (argc < 3) {
        return user_manual();
    }

    sqlite3 *db = NULL; // db connection

    if (create_conn(&db) != 0) {
        return 1;
    }

    // TODO tasks
    if (strcmp(argv[1], "todo") == 0) {
        if (strcmp(argv[2], "list") == 0) {
            if (list_tasks(db) != 0) {
                sqlite3_close(db);
                return 1;
            }
        } else if (strcmp(argv[2], "add") == 0) {
            if (argc < 4) {
                fprintf(stderr, "Missing task title for add\n");
                return 1;
            }

            char title[TITLE_SIZE] = "";

            for (int i = 3; i < argc; i++) {
                size_t needed =
                    strlen(title) + strlen(argv[i]) + (i > 3 ? 1 : 0) + 1;

                if (needed > sizeof(title)) {
                    fprintf(stderr, "Task title too long\n");
                    return 1;
                }

                if (i > 3) {
                    strcat(title, " ");
                }

                strcat(title, argv[i]);
            }

            add_task(db, title);
        }
    }

    sqlite3_close(db);
    return 0;
}

int user_manual() {
    printf("Usage: mytls <todo> <...>\n");
    return 1;
}
