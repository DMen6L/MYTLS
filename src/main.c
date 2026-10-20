#include "database.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
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
                sqlite3_close(db);
                return 1;
            }

            char title[TITLE_SIZE] = "";

            for (int i = 3; i < argc; i++) {
                size_t needed =
                    strlen(title) + strlen(argv[i]) + (i > 3 ? 1 : 0) + 1;

                if (needed > sizeof(title)) {
                    fprintf(stderr, "Task title too long\n");
                    sqlite3_close(db);
                    return 1;
                }

                if (i > 3) {
                    strcat(title, " ");
                }

                strcat(title, argv[i]);
            }

            add_task(db, title);
        } else if (strcmp(argv[2], "done") == 0) {
            if (argc > 4) {
                fprintf(stderr, "Too many arguments for the task <done>");
                sqlite3_close(db);
                return 1;
            }

            char *end;
            errno = 0;

            long value = strtol(argv[3], &end, 10);

            if (errno == ERANGE || end == argv[3] || *end != '\0' ||
                value <= 0 || value > INT_MAX) {
                fprintf(stderr, "Invalid task ID\n");
                sqlite3_close(db);
                return 1;
            }

            int id = (int)value;

            complete_task(db, id);
        }
    }

    sqlite3_close(db);
    return 0;
}

int user_manual() {
    printf("Usage: mytls <todo> <...>\n");
    return 1;
}
