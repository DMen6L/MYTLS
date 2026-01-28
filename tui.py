import curses
import time

test_todos = [""]
CTRL_B = 2
CTRL_F = 6
ESC = 27

def draw_todo(stdscr, header_win, options_win, h, w):
    stdscr.keypad(True)

    # Clearing the panel
    header_win.erase()
    options_win.erase()

    todo_win = curses.newwin(len(test_todos)+1, w-1, 1, 1)
    alert_win = curses.newwin(1, w-1, h-1, 1)
    
    options_win.refresh()

    curr_todo = 0

    while True:
        header_win.addstr(0, int(w/2)-10, "TO DO LIST")

        alert_win.addstr(0, 0, "^B to turn back")
        alert_win.addstr(0, 16, "^F for new task")

        for i, todo in enumerate(test_todos):
            todo_win.addstr(i, 1, f"{i}. {todo}")

        header_win.refresh()
        todo_win.refresh()
        alert_win.refresh()

        key = stdscr.getch()

        if key == -1:
            continue
        
        if key == CTRL_B:
            break

        if key == CTRL_F:
            y, x = todo_win.getmaxyx()
            todo_win.resize(y+1, x)

            test_todos.append("")
            curr_todo = len(test_todos)-1
            continue

        if key == curses.KEY_UP:
            if curr_todo > 0:
                curr_todo -= 1
            continue

        if key == curses.KEY_DOWN:
            if curr_todo < len(test_todos)-1:
                curr_todo += 1
            continue

        if key in (curses.KEY_BACKSPACE, 127, 8):
            test_todos[curr_todo] = test_todos[curr_todo][:-1]
            continue

        if 32 <= key <= 126:
            test_todos[curr_todo] += chr(key)
            
        todo_win.erase()
    
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

    curr_opt = 0 # CUrrent chosen option counter

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
                draw_todo(stdscr, header_win, options_win, h, w)
                curses.flushinp()

    stdscr.clear()