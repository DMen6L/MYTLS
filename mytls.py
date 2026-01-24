#!/usr/bin/env python3
import argparse
import curses
import time

# =================
# STANDARD COMMANDS
# =================
def greet(args):
    print(f"Hello, {args.name}")

# ====================
# INTERACTIVE COMMANDS
# ====================
def tui(stdscr):
    # TODO: add todolist, date+time work

    """
    Docstring for tui
    
    ? newpad(), newwin(), time.sleep()
    """

    GREET = "!!!! Welcome to MYTLS !!!!"
    GREET_LEN = len(GREET)

    h, w = stdscr.getmaxyx() # Height and width of the terminal

    welcome_pad = curses.newpad(1, w+GREET_LEN)

    i = 0

    while True:
        welcome_pad.clear()

        welcome_pad.addstr(0, i, GREET)

        if i > w-GREET_LEN:
            welcome_pad.addstr(0, 0, GREET[-(i+GREET_LEN-w):])

        i = (i+1) % w

        welcome_pad.refresh(0, 0, 0, 0, 0, w-1)

        time.sleep(0.1)

# ========
# CLI INIT
# ========
def main():
    # Main parser and CLI tool
    parser = argparse.ArgumentParser(
        prog="mytls",
        description="Personal CLI tool"
    )

    """
    Subparsers

    greet - test stage
    tui - interactive window work
    """
    subparsers = parser.add_subparsers(dest="command")

    greet_parser = subparsers.add_parser("greet", help="Greet user")
    greet_parser.add_argument("-n", "--name", required=True)
    greet_parser.set_defaults(func=greet)

    tui_parser = subparsers.add_parser("tui", help="TUI window for interactive tasks")
    tui_parser.set_defaults(func="tui")

    args = parser.parse_args() # Initialising parser

    if args.command == "tui":
        curses.wrapper(tui)
    elif hasattr(args, "func"):
        args.func(args)
    else:
        parser.print_help()
    

if __name__ == "__main__":
    main()