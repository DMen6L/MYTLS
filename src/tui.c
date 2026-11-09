#include "tui.h"
#include "database.h"

#include <curses.h>
#include <limits.h>
#include <form.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
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
        if (task->progress >= task->goal)
            return;
        Task candidate = *task;
        int progress = task->progress + 1;
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
        Task candidate = {.id = -1, .progress = 0, .goal = 1};
        if (prompt_new_task(&candidate, error) != 1)
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

static void task_status(const Task *task, char *out, size_t capacity) {
    if (task->goal <= 0 || task->progress < 0) {
        snprintf(out, capacity, "INVALID");
    } else if (task->goal == 1) {
        snprintf(out, capacity, "%s", task_complete(task) ? "DONE" : "PENDING");
    } else {
        int filled = task->progress >= task->goal ? 12
            : (int)((int64_t)task->progress * 12 / task->goal);
        char bar[13];
        for (int i = 0; i < 12; i++)
            bar[i] = i < filled ? '#' : '-';
        bar[12] = '\0';
        snprintf(out, capacity, "[%s] %d/%d", bar, task->progress, task->goal);
    }
}

/* Limit every line to the screen width; curses must not wrap table cells. */
static void draw_line(int row, const char *text) {
    if (row >= 0 && row < LINES && COLS > 3)
        mvaddnstr(row, 2, text, COLS - 3);
}

void list_tasks(const TaskList *tasks, size_t selected) {
    draw_line(1, "TASK MANAGER");
    draw_line(3, "ID    TASK                 STATUS");
    if (LINES > 4 && COLS > 4)
        mvhline(4, 2, ACS_HLINE, COLS - 4);

    for (size_t i = 0; i < tasks->count && (int)i + 5 < LINES - 2; i++) {
        if (i == selected)
            attron(A_REVERSE);
        const Task *task = &tasks->items[i];
        char status[64], line[128];
        task_status(task, status, sizeof(status));
        snprintf(line, sizeof(line), "%-5zu %-20.20s %s", i + 1, task->title, status);
        draw_line((int)i + 5, line);
        if (i == selected)
            attroff(A_REVERSE);
    }
    draw_line(LINES - 2, "UP/DOWN: NAVIGATE | a: ADD | x: +1 PROGRESS | d: DELETE | q: QUIT");
    refresh();
}

/* Forms pad field buffers with spaces. Strip that padding before validation. */
static int read_field(FIELD *field, char *out, size_t capacity, Error *error) {
    const char *text = field_buffer(field, 0);
    if (text == NULL) {
        snprintf(error->message, sizeof(error->message), "Reading form field failed");
        return 1;
    }
    size_t length = strlen(text);
    while (length > 0 && isspace((unsigned char)text[length - 1]))
        length--;
    if (length >= capacity) {
        snprintf(error->message, sizeof(error->message), "Field value is too long");
        return 1;
    }
    memcpy(out, text, length);
    out[length] = '\0';
    return 0;
}

static int read_task_form(FIELD **fields, Task *out, Error *error) {
    char title[TITLE_SIZE], numbers[2][33];
    if (read_field(fields[0], title, sizeof(title), error) != 0)
        return 1;
    const char *text = title;
    while (isspace((unsigned char)*text))
        text++;
    if (*text == '\0') {
        snprintf(error->message, sizeof(error->message), "Title must not be blank");
        return 1;
    }
    int values[2];
    for (int i = 0; i < 2; i++) {
        if (read_field(fields[i + 1], numbers[i], sizeof(numbers[i]), error) != 0)
            return 1;
        char *end;
        errno = 0;
        long value = strtol(numbers[i], &end, 10);
        if (errno == ERANGE || end == numbers[i] || *end != '\0' ||
            value < (i == 0 ? 0 : 1) || value > INT_MAX) {
            snprintf(error->message, sizeof(error->message),
                     "%s must be an integer from %d to %d",
                     i == 0 ? "Initial progress" : "Goal", i == 0 ? 0 : 1, INT_MAX);
            return 1;
        }
        values[i] = (int)value;
    }
    Task candidate = {.id = -1};
    if (asgn_task_val(candidate.title, sizeof(candidate.title), title, VALUE_TEXT, error) ||
        asgn_task_val(&candidate.progress, sizeof(candidate.progress), &values[0], VALUE_INT, error) ||
        asgn_task_val(&candidate.goal, sizeof(candidate.goal), &values[1], VALUE_INT, error))
        return 1;
    *out = candidate;
    return 0;
}

int prompt_new_task(Task *out, Error *error) {
    error->message[0] = '\0';
    if (out == NULL) {
        snprintf(error->message, sizeof(error->message), "prompt_new_task: output task is required");
        return -1;
    }
    FIELD *fields[4] = {0};
    FORM *form = NULL;
    WINDOW *win = NULL, *sub = NULL;
    int result = -1, posted = 0;
    int previous_cursor = curs_set(1);
    Error validation = {0};

    for (int i = 0; i < 3; i++) {
        fields[i] = new_field(1, i == 0 ? 36 : 12, i * 2, 0, 0, 0);
        if (fields[i] == NULL)
            goto failure;
        if (field_opts_off(fields[i], O_AUTOSKIP | O_STATIC) != E_OK ||
            set_max_field(fields[i], i == 0 ? TITLE_SIZE - 1 : 32) != E_OK ||
            set_field_back(fields[i], A_UNDERLINE) != E_OK)
            goto failure;
    }
    if (set_field_buffer(fields[1], 0, "0") != E_OK ||
        set_field_buffer(fields[2], 0, "1") != E_OK)
        goto failure;
    form = new_form(fields);
    if (form == NULL)
        goto failure;

    for (;;) {
        if (LINES < 12 || COLS < 60) {
            erase();
            draw_line(0, "Resize to at least 60 columns x 12 rows; Esc cancels");
            refresh();
            int ch = getch();
            if (ch == 27) {
                result = 0;
                break;
            }
            if (ch == ERR)
                goto failure;
            continue;
        }
        if (win == NULL) {
            win = newwin(12, 60, (LINES - 12) / 2, (COLS - 60) / 2);
            if (win == NULL)
                goto failure;
            sub = derwin(win, 5, 36, 3, 20);
            if (sub == NULL || keypad(win, TRUE) == ERR ||
                set_form_win(form, win) != E_OK || set_form_sub(form, sub) != E_OK ||
                post_form(form) != E_OK)
                goto failure;
            posted = 1;
            box(win, 0, 0);
            mvwaddstr(win, 1, 2, "NEW TASK");
            mvwaddstr(win, 3, 2, "Title:");
            mvwaddstr(win, 5, 2, "Initial progress:");
            mvwaddstr(win, 7, 2, "Goal:");
            mvwaddstr(win, 10, 2, "Tab/Up/Down: field | Enter: save | Esc: cancel");
        }
        mvwhline(win, 9, 2, ' ', 56);
        mvwaddnstr(win, 9, 2, validation.message, 56);
        pos_form_cursor(form);
        wrefresh(win);
        int ch = wgetch(win);
        if (ch == ERR)
            goto failure;
        if (ch == 27) {
            result = 0;
            break;
        }
        if (ch == KEY_RESIZE) {
            if (form_driver(form, REQ_VALIDATION) != E_OK || unpost_form(form) != E_OK)
                goto failure;
            posted = 0;
            set_form_sub(form, NULL);
            set_form_win(form, NULL);
            delwin(sub);
            delwin(win);
            sub = win = NULL;
            erase();
            refresh();
            continue;
        }
        if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
            if (form_driver(form, REQ_VALIDATION) != E_OK)
                goto failure;
            if (read_task_form(fields, out, &validation) == 0) {
                result = 1;
                break;
            }
            continue;
        }
        int request;
        switch (ch) {
        case '\t': case KEY_DOWN: request = REQ_NEXT_FIELD; break;
        case KEY_BTAB: case KEY_UP: request = REQ_PREV_FIELD; break;
        case KEY_LEFT: request = REQ_PREV_CHAR; break;
        case KEY_RIGHT: request = REQ_NEXT_CHAR; break;
        case KEY_HOME: request = REQ_BEG_FIELD; break;
        case KEY_END: request = REQ_END_FIELD; break;
        case KEY_BACKSPACE: case 127: case 8: request = REQ_DEL_PREV; break;
        case KEY_DC: request = REQ_DEL_CHAR; break;
        case 21: request = REQ_CLR_FIELD; break; /* Ctrl+U */
        default:
            if (ch < 32 || ch > 126)
                continue;
            request = ch;
        }
        int rc = form_driver(form, request);
        if (rc == E_REQUEST_DENIED) {
            snprintf(validation.message, sizeof(validation.message), "Field boundary or length limit reached");
        } else if (rc != E_OK) {
            goto failure;
        } else {
            validation.message[0] = '\0';
        }
    }
    goto cleanup;

failure:
    snprintf(error->message, sizeof(error->message), "prompt_new_task: creating or operating task form failed");
cleanup:
    if (posted)
        unpost_form(form);
    if (form != NULL)
        free_form(form);
    for (int i = 0; i < 3; i++)
        if (fields[i] != NULL)
            free_field(fields[i]);
    if (sub != NULL)
        delwin(sub);
    if (win != NULL)
        delwin(win);
    if (previous_cursor != ERR)
        curs_set(previous_cursor);
    touchwin(stdscr);
    refresh();
    return result;
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
