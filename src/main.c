#include "database.h"
#include "task.h"

#include <curses.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    TaskList tasks;
    size_t selected;
    int running;
} TuiState;

int user_manual(); // Print user manual
void parse_controlch(sqlite3 *db, TuiState *state, int ch);

int cli_run(sqlite3 *db, int argc, char *argv[]);
int tui_run(sqlite3 *db);
int list_tasks(TaskList *tasks, int selected);
int prompt_new_task(char *title, size_t capacity);

int main(int argc, char *argv[]) {
    sqlite3 *db = NULL; // db connection

    if (create_conn(&db) != 0) {
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

void parse_controlch(sqlite3 *db, TuiState *state, int ch) {
    // If q is pressed stop the TUI
    if (ch == 'q')
        state->running = 0;
    // If key up is pressed or k is pressed and when it is appropriate move
    // selected up
    else if ((ch == KEY_UP || ch == 'k') && state->tasks.count > 0 &&
             state->selected > 0)
        state->selected--;
    // The same move logic but down
    else if ((ch == KEY_DOWN || ch == 'j') &&
             state->selected < state->tasks.count - 1)
        state->selected++;
    // Toggle done/undone
    else if (ch == 'd' || ch == 'u') {
        if (state->selected >= state->tasks.count) {
            return;
        }

        Task *task = &state->tasks.items[state->selected];
        int desired_done = (ch == 'd');

        if (task->done != desired_done) {
            if (set_task_done(db, task->id, !task->done) == 0)
                task->done = !task->done;
        }
    }
    // Add new task
    else if (ch == 'a') {
        char title[TITLE_SIZE] = {0};

        if (prompt_new_task(title, TITLE_SIZE) == 0) {
            Task new_task = {.id = -1, .done = 0};

            strcpy(new_task.title, title);

            if (add_task(db, &new_task) == 0 &&
                state->tasks.count < PAGE_SIZE) {
                state->tasks.items[state->tasks.count] = new_task;
                state->tasks.count++;
            }
        }
    }
}

int cli_run(sqlite3 *db, int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "tui") == 0) {
        return tui_run(db);
    }

    return user_manual();
}

int tui_run(sqlite3 *db) {
    // Initialize TUI state
    TuiState state = {.tasks = {0}, .selected = 0, .running = 1};

    if (fetch_tasks(db, 0, PAGE_SIZE, &state.tasks) != 0) {
        fprintf(stderr, "Could not load tasks\n");
        return 1;
    }

    if (initscr() == NULL) {
        task_list_free(&state.tasks);
        fprintf(stderr, "Could not initialize the curses screen");
        return 1;
    }

    cbreak();             // Receive input without pressing Enter
    noecho();             // Don't print typed keys automatically
    keypad(stdscr, TRUE); // Recognize arrow keys
    curs_set(0);

    while (state.running) {
        erase();
        list_tasks(&state.tasks, state.selected);
        int ch = getch();

        parse_controlch(db, &state, ch);
    }

    endwin();
    task_list_free(&state.tasks);

    return 0;
}

int list_tasks(TaskList *tasks, int selected) {
    mvprintw(1, 2, "TASK MANAGER");
    mvprintw(3, 2, "%-5s %-20s %-10s", "ID", "TASK", "STATUS");

    mvhline(4, 2, ACS_HLINE, 40);

    for (size_t i = 0; i < tasks->count; i++) {
        if (i == selected)
            attron(A_REVERSE);

        Task *task = &tasks->items[i];

        mvprintw(5 + i, 2, "%-5d %-20.20s %-10s", task->id, task->title,
                 task->done ? "Done" : "Pending");

        if (selected == i)
            attroff(A_REVERSE);
    }

    mvprintw(tasks->count + 6, 2, "UP/DOWN: NAVIGATE | q: QUIT");
    refresh();

    return 0;
}

int prompt_new_task(char *title, size_t capacity) {
    if (capacity < 2 || capacity >= INT_MAX)
        return 1;

    int height = 5;
    int width = 50;
    int y = (LINES - height) / 2;
    int x = (COLS - width) / 2;

    WINDOW *win = newwin(height, width, y, x);
    if (!win)
        return 1;

    box(win, 0, 0);
    mvwprintw(win, 1, 2, "New task title:");
    wmove(win, 2, 2);

    echo();
    curs_set(1);

    int result = wgetnstr(win, title, (int)capacity - 1);

    noecho();
    curs_set(0);

    delwin(win);
    touchwin(stdscr);
    refresh();

    return result == OK && title[0] != '\0' ? 0 : 1;
}
