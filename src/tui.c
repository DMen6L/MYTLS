#include "tui.h"
#include "database.h"
#include "task.h"

#include <ctype.h>
#include <curses.h>
#include <errno.h>
#include <form.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void progress_bar(int progress, int goal, char bar[13]) {
    int filled = 0;

    if (goal > 0) {
        double ratio = (double)progress / goal;

        if (ratio < 0.0)
            ratio = 0.0;
        if (ratio > 1.0)
            ratio = 1.0;

        filled = (int)(ratio * 10);
    }

    bar[0] = '[';

    for (int i = 0; i < 10; i++) {
        bar[i + 1] = i < filled ? '#' : '-';
    }

    bar[11] = ']';
    bar[12] = '\0';
}

int draw_tasks(TuiState *state) {
    int startY = 3;
    TaskList *tasks = &state->tasks;

    // Headers
    mvprintw(startY, 2, "%-5s %-30s %-14s", "ID", "TASK", "STATUS");
    mvhline(startY + 1, 2, ACS_HLINE, 68);

    // Draw tasks table
    for (size_t i = 0; i < tasks->count; i++) {
        Task *task = &tasks->items[i];

        // Active task show
        if (i == state->selected)
            attron(A_REVERSE);

        if (task->goal == 1 && task->progress < task->goal) {
            mvprintw(startY + 2 + (int)i, 2, "%-5d %-30.30s %-8.8s", i + 1,
                     task->title, "PENDING");
        } else if (task->progress < task->goal && task->goal > 1) {
            char bar[13];
            progress_bar(task->progress, task->goal, bar);

            mvprintw(startY + 2 + (int)i, 2, "%-5d %-30.30s %-14s %d/%d", i + 1,
                     task->title, bar, task->progress, task->goal);
        } else {
            mvprintw(startY + 2 + (int)i, 2, "%-5d %-30.30s %-8.8s", i + 1,
                     task->title, "DONE");
        }

        // Clean attrubute
        if (i == state->selected)
            attroff(A_REVERSE);
    }

    return 0;
}

int prompt_new_task(sqlite3 *db, Task *newTask, Error *err) {
    const int height = 11;
    const int width = 54;

    if (LINES < height || COLS < width) {
        snprintf(err->message, sizeof(err->message),
                 "Terminal too small for task popup");
        return 1;
    }

    int y = (LINES - height) / 2;
    int x = (COLS - width) / 2;

    WINDOW *win = newwin(height, width, y, x);
    if (!win)
        return 1;

    keypad(win, TRUE);

    char title[256] = {0};
    char goal_text[16] = {0};

    box(win, 0, 0);
    mvwprintw(win, 1, 2, "Add New Task");

    mvwprintw(win, 3, 2, "Title:");
    mvwprintw(win, 6, 2, "Goal:");

    mvwprintw(win, 9, 2, "Enter: Save");
    wrefresh(win);

    // Enable typing
    echo();
    curs_set(1);

    // Read title
    wmove(win, 4, 2);
    int rc = wgetnstr(win, title, sizeof(title) - 1);

    if (rc == ERR || title[0] == '\0')
        goto fail;

    // Read goal
    wmove(win, 7, 2);
    wrefresh(win);

    rc = wgetnstr(win, goal_text, sizeof(goal_text) - 1);

    if (rc == ERR)
        goto fail;

    // Parse goal
    char *end;
    errno = 0;
    long goal = strtol(goal_text, &end, 10);

    if (errno == ERANGE || end == goal_text || *end != '\0' || goal < 1 ||
        goal > INT_MAX) {
        snprintf(err->message, sizeof(err->message), "Invalid task goal");
        goto fail;
    }

    newTask->goal = (int)goal;
    snprintf(newTask->title, sizeof(newTask->title), "%s", title);

    // Prepare INSERT values
    const char *cols[] = {"title", "progress", "goal"};
    const void *vals[] = {newTask->title, &newTask->progress, &newTask->goal};
    const ValueType types[] = {VALUE_TEXT, VALUE_INT, VALUE_INT};

    rc = add_val(db, "tasks", cols, vals, types, 3, &newTask->id, err);

    if (rc != 0)
        goto fail;

    noecho();
    curs_set(0);
    delwin(win);
    touchwin(stdscr);
    refresh();

    return 0;

fail:
    noecho();
    curs_set(0);
    delwin(win);
    touchwin(stdscr);
    refresh();
    return 1;
}

void handle_key(sqlite3 *db, TuiState *state, const int ch, Error *err) {
    switch (ch) {
    case 'k':
    case KEY_UP:
        if (state->selected > 0)
            state->selected--;
        break;
    case 'j':
    case KEY_DOWN:
        if (state->selected + 1 < state->tasks.count)
            state->selected++;
        break;
    case 'a':
        Task newTask = {.id = -1, .progress = 0, .goal = 1};
        if (prompt_new_task(db, &newTask, err) == 0 &&
            state->tasks.count < PAGE_SIZE) {
            state->tasks.items[state->tasks.count] = newTask;
            state->tasks.count++;
        }
        break;
    case 'q':
        state->running = 0;
        break;
    default:
        break;
    }
}

int tui_run(sqlite3 *db, TuiState *state, Error *err) {
    if (initscr() == NULL) {
        snprintf(err->message, sizeof(err->message),
                 "tui: screen initialization error");
        return 1;
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    const char *cols[] = {"id", "title", "progress", "goal"};

    fetch_table(db, "tasks", cols, &state->tasks, 4, err);
    state->running = 1;

    while (state->running) {
        erase();

        box(stdscr, 0, 0);
        mvprintw(1, 2, "Task Manager");
        draw_tasks(state);

        refresh();

        int ch = getch();

        handle_key(db, state, ch, err);

        if (err->message[0] != '\0') {
            state->running = 0;
        }
    }

    endwin();
    return free_table_list(&state->tasks, "tasks", err);
}
