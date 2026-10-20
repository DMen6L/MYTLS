#include "database.h"

#include <stdio.h>
#include <string.h>

void user_manual(); // Print user manual

int main(int argc, char *argv[]) {
    sqlite3 *db = NULL; // db connection

    if (create_conn(&db) != 0) {
        return 1;
    }

    printf("Database connection created successfully!\n");

    sqlite3_close(db);
    return 0;
}

void user_manual() { printf("Usage: mytls <todo> <...>\n"); }
