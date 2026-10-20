#include <stdio.h>
#include <string.h>

void user_manual(); // Print user manual

int main(int argc, char *argv[]) {
    if (argc < 3) {
        user_manual();
        return 1;
    }

    if (strcmp(argv[1], "todo") == 0) {
        if (strcmp(argv[2], "list") == 0) {
            FILE *file = fopen("tasks.txt", "r");

            if (file == NULL) {
                perror("fopen");
                return 1;
            }

            char line[256];

            while (fgets(line, sizeof(line), file) != NULL) {
                printf("%s", line);
            }

            if (ferror(file)) {
                perror("fgets");
                fclose(file);
                return 1;
            }

            fclose(file);
        }
    }

    return 0;
}

void user_manual() { printf("Usage: mytls <todo> <...>\n"); }
