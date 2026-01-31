import curses
from typing import List

CTRL_B = 2
CTRL_F = 6
ESC = 27

# Get and save todo tasks
def get_file_data(path: str):
    try:
        with open(path, "r") as f:
            return [line.strip() for line in f]
    except FileNotFoundError:
        return []
def save_to_file(task_list: List[str], path: str):
    with open(path, "w") as f:
        for el in task_list:
            f.write(el + '\n')
    

def draw_todo(stdscr, header_win, options_win, h, w, tasks):
    stdscr.keypad(True)

    # Clearing the panel
    header_win.erase()
    options_win.erase()

    tasks_win = curses.newwin(len(tasks)+1, w-1, 1, 1)
    alert_win = curses.newwin(1, w-1, h-1, 1)
    
    options_win.refresh()

    curr_task = 0

    while True:
        header_win.addstr(0, int(w/2)-10, "TO DO LIST")

        alert_win.addstr(0, 0, "^B to turn back")
        alert_win.addstr(0, 16, "^F for new task")

        for i, task in enumerate(tasks):
            tasks_win.addstr(i, 1, f"{i}. {task}")

        header_win.refresh()
        tasks_win.refresh()
        alert_win.refresh()

        # Key control
        key = stdscr.getch()

        if key == -1:
            continue
        
        # Go back to main
        if key == CTRL_B:
            save_to_file(tasks, "todo_tasks.txt")
            break

        # Add new task
        if key == CTRL_F:
            y, x = tasks_win.getmaxyx()
            tasks_win.resize(y+1, x)

            tasks.append("")
            curr_task = len(tasks)-1
            continue

        # Move up the list
        if key == curses.KEY_UP:
            if curr_task > 0:
                curr_task -= 1
            continue

        # Move down the list
        if key == curses.KEY_DOWN:
            if curr_task < len(tasks)-1:
                curr_task += 1
            continue
        
        if key in (curses.KEY_BACKSPACE, 127, 8):
            tasks[curr_task] = tasks[curr_task][:-1]
            continue

        if 32 <= key <= 126:
            tasks[curr_task] += chr(key)
            
        tasks_win.erase()
    
    stdscr.clear()
    

def run_tui(stdscr):
    curses.start_color()
    curses.init_pair(1, curses.COLOR_BLACK, curses.COLOR_WHITE)

    CHOSEN_OPT = curses.color_pair(1) # Chosen option color scheme

    OPTIONS = [
        "TODO",
        "Homeworks",
        "Kafedra"
    ]

    h, w = stdscr.getmaxyx()

    header_win = curses.newwin(1, w-1, 0, 0)
    options_win = curses.newwin(len(OPTIONS), w-1, 1, 1)

    # Settings for smooth terminal display
    stdscr.nodelay(True)
    stdscr.timeout(100)
    stdscr.keypad(True)
    stdscr.clear()

    curr_opt = 0 # Current chosen option counter

    # stdscr.addstr(h-1, w-19, "press 'q' to leave", CHOSEN_OPT | curses.A_BOLD)

    while True:
        header_win.addstr(0, int(w/2)-7, "Welcome")

        for i, option in enumerate(OPTIONS):
            attr = CHOSEN_OPT if i == curr_opt else curses.A_NORMAL
            options_win.addstr(i, 0, option, attr)

        header_win.refresh()
        options_win.refresh()

        key = stdscr.getch()

        if key == -1:
            continue
        
        if key == ESC:
            break
        elif key == curses.KEY_DOWN and curr_opt < len(OPTIONS) - 1:
            curr_opt += 1
        elif key == curses.KEY_UP and curr_opt > 0:
            curr_opt -= 1
        elif key in (curses.KEY_ENTER, 10, 13):
            opt = OPTIONS[curr_opt]

            if opt == "TODO":
                todos = get_file_data("todo_tasks.txt")
                draw_todo(stdscr, header_win, options_win, h, w, todos)
                curses.flushinp()

    stdscr.clear()