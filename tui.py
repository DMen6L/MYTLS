import curses
import time

def draw_todo(stdscr, header_win, options_win, h, w):
    # Clearing the panel
    header_win.erase()
    options_win.erase()
    
    options_win.refresh()

    while True:
        header_win.addstr(0, int(w/2)-10, "TO DO LIST")
        header_win.refresh()

        try:
            key = stdscr.getkey()
        except curses.error:
            key = None
        
        if key == "b":
            break
    
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

        try:
            key = stdscr.getkey()
        except:
            key = None
        
        if key == "q":
            break
        elif key == "KEY_DOWN" and curr_opt < len(OPTIONS)-1:
            curr_opt += 1
        elif key == "KEY_UP" and curr_opt > 0:
            curr_opt -= 1
        elif key == "\n":
            opt = OPTIONS[curr_opt]

            if opt == "TODO":
                draw_todo(stdscr, header_win, options_win, h, w)

    stdscr.clear()