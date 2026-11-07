#include "tui.h"
#include "database.h"

#include <curses.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int tui_run(sqlite3 *db, TuiState *state) {
    fetch_tasks(db, 0, PAGE_SIZE, &state->tasks, state->err_txt,
                sizeof(state->err_txt));

    if (state->err_txt[0] != '\0')
        return 1;

    if (initscr() == NULL) {
        task_list_free(&state->tasks);
        snprintf(state->err_txt, sizeof(state->err_txt),
                 "Could not initialize the curses screen");
        return 1;
    }

    if (cbreak() == ERR || noecho() == ERR || keypad(stdscr, TRUE) == ERR) {
        snprintf(state->err_txt, sizeof(state->err_txt),
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
            snprintf(state->err_txt, sizeof(state->err_txt),
                     "tui_run: reading keyboard input failed");
        } else {
            parse_controlch(db, state, ch);
        }

        if (state->err_txt[0] != '\0')
            state->running = 0;
    }

    if (endwin() == ERR && state->err_txt[0] == '\0') {
        snprintf(state->err_txt, sizeof(state->err_txt),
                 "tui_run: restoring terminal failed");
    }
    task_list_free(&state->tasks);

    return state->err_txt[0] == '\0' ? 0 : 1;
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
    else if ((ch == KEY_DOWN || ch == 'j') && state->tasks.count > 0 &&
             state->selected < state->tasks.count - 1)
        state->selected++;
    // Toggle done/undone
    else if (ch == 'x') {
        if (state->selected >= state->tasks.count) {
            return;
        }

        Task *task = &state->tasks.items[state->selected];
        set_task_done(db, task->id, !task->done, state->err_txt,
                      sizeof(state->err_txt));
        if (state->err_txt[0] == '\0')
            task->done = !task->done;
    }
    // Add new task
    else if (ch == 'a') {
        char title[TITLE_SIZE] = {0};

        prompt_new_task(title, TITLE_SIZE, state->err_txt,
                        sizeof(state->err_txt));

        if (state->err_txt[0] != '\0') {
            return;
        }

        Task new_task = {.id = -1, .done = 0};

        strcpy(new_task.title, title);

        /* Reserve space before inserting so an allocation failure cannot lose
         * the new task from the displayed list. */
        if (state->tasks.count < PAGE_SIZE && state->tasks.items == NULL) {
            state->tasks.items =
                malloc(PAGE_SIZE * sizeof(*state->tasks.items));
            if (state->tasks.items == NULL) {
                snprintf(state->err_txt, sizeof(state->err_txt),
                         "parse_controlch: allocating task buffer failed");
                return;
            }
        }

        add_task(db, &new_task, state->err_txt, sizeof(state->err_txt));

        if (state->tasks.count < PAGE_SIZE && state->err_txt[0] == '\0') {
            state->tasks.items[state->tasks.count] = new_task;
            state->tasks.count++;
        }
    } else if (ch == 'd') {
        if (state->selected >= state->tasks.count)
            return;
        Task *task = &state->tasks.items[state->selected];

        prompt_task_del(db, task, state->err_txt, sizeof(state->err_txt));

        // If task is deleted shift it to the left
        if (task->id == -1) {
            size_t index = state->selected;

            for (size_t i = index; i + 1 < state->tasks.count; i++) {
                state->tasks.items[i] = state->tasks.items[i + 1];
            }

            state->tasks.count--;

            if (state->selected >= state->tasks.count && state->selected > 0) {
                state->selected--;
            }
        }
    }
}

void list_tasks(TaskList *tasks, int selected) {
    mvprintw(1, 2, "TASK MANAGER");
    mvprintw(3, 2, "%-5s %-20s %-10s", "ID", "TASK", "STATUS");

    mvhline(4, 2, ACS_HLINE, 40);

    for (size_t i = 0; i < tasks->count; i++) {
        if (i == selected)
            attron(A_REVERSE);

        Task *task = &tasks->items[i];

        mvprintw(5 + i, 2, "%-5d %-20.20s %-10s", i + 1, task->title,
                 task->done ? "Done" : "Pending");

        if (selected == i)
            attroff(A_REVERSE);
    }

    mvprintw(tasks->count + 6, 2, "UP/DOWN: NAVIGATE | q: QUIT");
    refresh();
}

void prompt_new_task(char *title, size_t capacity, char *err_txt,
                     size_t err_capacity) {
    if (err_capacity > 0)
        err_txt[0] = '\0';
    if (title == NULL || capacity < 2 || capacity >= INT_MAX) {
        snprintf(err_txt, err_capacity,
                 "prompt_new_task: title buffer or capacity is invalid");
        return;
    }

    int height = 5;
    int width = 50;
    int y = (LINES - height) / 2;
    int x = (COLS - width) / 2;

    WINDOW *win = newwin(height, width, y, x);
    if (!win) {
        snprintf(err_txt, err_capacity,
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
        snprintf(err_txt, err_capacity,
                 "prompt_new_task: reading title failed");
    }
}

void prompt_task_del(sqlite3 *db, Task *task, char *err_txt,
                     size_t err_capacity) {
    if (err_capacity > 0)
        err_txt[0] = '\0';
    int height = 5;
    int width = 50;
    int y = (LINES - height) / 2;
    int x = (COLS - width) / 2;

    WINDOW *win = newwin(height, width, y, x);
    if (!win) {
        snprintf(err_txt, err_capacity,
                 "prompt_task_del: creating confirmation window failed");
        return;
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
        snprintf(err_txt, err_capacity,
                 "prompt_task_del: reading confirmation failed");
    }

    if (choice == 'y') {
        del_task(db, task->id, err_txt, err_capacity);
        if (err_txt[0] == '\0')
            task->id = -1;
    }

    delwin(win);

    touchwin(stdscr);
    refresh();
}
