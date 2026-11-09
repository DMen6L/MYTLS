#include "tui.h"
#include "database.h"

#include <curses.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

static int task_complete(const Task *task) {
    return task->goal > 0 && task->progress >= task->goal;
}

/* Keep the current list until a complete replacement is available. */
static int reload_tasks(sqlite3 *db, TuiState *state,
                         sqlite3_int64 preferred_id, Error *error) {
    TaskList loaded = {0};
    fetch_tasks(db, 0, PAGE_SIZE, &loaded, error);
    if (error->message[0] != '\0')
        return 1;

    size_t selected = state->selected;
    if (loaded.count == 0)
        selected = 0;
    else if (selected >= loaded.count)
        selected = loaded.count - 1;

    for (size_t i = 0; i < loaded.count; i++) {
        if (loaded.items[i].id == preferred_id) {
            selected = i;
            break;
        }
    }

    task_list_free(&state->tasks);
    state->tasks = loaded;
    state->selected = selected;
    return 0;
}

int tui_run(sqlite3 *db, TuiState *state, Error *error) {
    if (reload_tasks(db, state, -1, error) != 0)
        return 1;

    if (initscr() == NULL) {
        task_list_free(&state->tasks);
        snprintf(error->message, sizeof(error->message),
                 "Could not initialize the curses screen");
        return 1;
    }

    if (cbreak() == ERR || noecho() == ERR || keypad(stdscr, TRUE) == ERR) {
        snprintf(error->message, sizeof(error->message),
                 "tui_run: configuring terminal input failed");
        endwin();
        task_list_free(&state->tasks);
        return 1;
    }
    curs_set(0);

    while (state->running) {
        erase();
        list_tasks(&state->tasks, state->selected);
        int ch = getch();

        if (ch == ERR) {
            snprintf(error->message, sizeof(error->message),
                     "tui_run: reading keyboard input failed");
        } else {
            parse_controlch(db, state, ch, error);
        }

        if (error->message[0] != '\0')
            state->running = 0;
    }

    if (endwin() == ERR && error->message[0] == '\0') {
        snprintf(error->message, sizeof(error->message),
                 "tui_run: restoring terminal failed");
    }
    task_list_free(&state->tasks);

    return error->message[0] == '\0' ? 0 : 1;
}

void parse_controlch(sqlite3 *db, TuiState *state, int ch, Error *error) {
    sqlite3_int64 preferred_id = -1;

    if (ch == 'q') {
        state->running = 0;
        return;
    } else if (ch == KEY_UP || ch == 'k') {
        if (state->selected > 0)
            state->selected--;
        return;
    } else if (ch == KEY_DOWN || ch == 'j') {
        if (state->tasks.count > 0 && state->selected < state->tasks.count - 1)
            state->selected++;
        return;
    } else if (ch == 'x') {
        if (state->selected >= state->tasks.count)
            return;

        const Task *task = &state->tasks.items[state->selected];
        Task candidate = *task;
        int progress = task_complete(task) ? 0 : task->goal;
        if (asgn_task_val(&candidate.progress, sizeof(candidate.progress),
                          &progress, VALUE_INT, error) != 0)
            return;

        const char *cols[] = {"progress"};
        const ValueType types[] = {VALUE_INT};
        const void *values[] = {&candidate.progress};
        if (update_task(db, task->id, cols, types, values,
                        sizeof(cols) / sizeof(cols[0]), error) != 0)
            return;
        preferred_id = task->id;
    } else if (ch == 'a') {
        char title[TITLE_SIZE] = {0};
        prompt_new_task(title, sizeof(title), error);
        if (error->message[0] != '\0')
            return;

        Task candidate = {.id = -1, .progress = 0, .goal = 1};
        if (asgn_task_val(candidate.title, sizeof(candidate.title),
                          title, VALUE_TEXT, error) != 0)
            return;

        const char *cols[] = {"title", "progress", "goal"};
        const ValueType types[] = {VALUE_TEXT, VALUE_INT, VALUE_INT};
        const void *values[] = {candidate.title, &candidate.progress, &candidate.goal};
        if (add_task(db, cols, types, values, sizeof(cols) / sizeof(cols[0]),
                     &preferred_id, error) != 0)
            return;
    } else if (ch == 'd') {
        if (state->selected >= state->tasks.count)
            return;

        const Task *task = &state->tasks.items[state->selected];
        if (prompt_task_del(task, error) != 1)
            return;
        del_task(db, task->id, error);
        if (error->message[0] != '\0')
            return;
    } else {
        return;
    }

    if (reload_tasks(db, state, preferred_id, error) != 0) {
        Error refresh_error = *error;
        snprintf(error->message, sizeof(error->message),
                 "Change saved; refreshing tasks failed: %.450s", refresh_error.message);
    }
}

void list_tasks(const TaskList *tasks, size_t selected) {
    mvprintw(1, 2, "TASK MANAGER");
    mvprintw(3, 2, "%-5s %-20s %-10s", "ID", "TASK", "STATUS");

    mvhline(4, 2, ACS_HLINE, 40);

    for (size_t i = 0; i < tasks->count; i++) {
        if (i == selected)
            attron(A_REVERSE);

        const Task *task = &tasks->items[i];

        mvprintw(5 + i, 2, "%-5zu %-20.20s %-10s", i + 1, task->title,
                 task_complete(task) ? "DONE" : "PENDING");

        if (selected == i)
            attroff(A_REVERSE);
    }

    mvprintw(tasks->count + 6, 2, "UP/DOWN: NAVIGATE | q: QUIT");
    refresh();
}

void prompt_new_task(char *title, size_t capacity, Error *error) {
    error->message[0] = '\0';
    if (title == NULL || capacity < 2 || capacity >= INT_MAX) {
        snprintf(error->message, sizeof(error->message),
                 "prompt_new_task: title buffer or capacity is invalid");
        return;
    }

    int height = 5;
    int width = 50;
    int y = (LINES - height) / 2;
    int x = (COLS - width) / 2;

    WINDOW *win = newwin(height, width, y, x);
    if (!win) {
        snprintf(error->message, sizeof(error->message),
                 "prompt_new_task: creating input window failed");
        return;
    }

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

    if (result != OK) {
        snprintf(error->message, sizeof(error->message),
                 "prompt_new_task: reading title failed");
    }
}

int prompt_task_del(const Task *task, Error *error) {
    error->message[0] = '\0';
    int height = 5;
    int width = 50;
    int y = (LINES - height) / 2;
    int x = (COLS - width) / 2;

    WINDOW *win = newwin(height, width, y, x);
    if (!win) {
        snprintf(error->message, sizeof(error->message),
                 "prompt_task_del: creating confirmation window failed");
        return -1;
    }

    box(win, 0, 0);
    mvwprintw(win, 1, 2, "Are you sure you wanna delete this task?");
    mvwprintw(win, 2, 2, "%.40s (y/n)", task->title);

    wrefresh(win);

    int choice;

    do {
        choice = wgetch(win);
    } while (choice != 'y' && choice != 'n' && choice != ERR);

    if (choice == ERR) {
        snprintf(error->message, sizeof(error->message),
                 "prompt_task_del: reading confirmation failed");
    }

    delwin(win);

    touchwin(stdscr);
    refresh();
    return choice == ERR ? -1 : choice == 'y';
}
