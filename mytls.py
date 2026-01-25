#!/usr/bin/env python3
import argparse
import curses
import time
from dataclasses import dataclass

@dataclass
class Choices:
    id: int
    text: str = ""
    marked: bool = False

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

    curses.init_pair(1, curses.COLOR_BLACK, curses.COLOR_WHITE)

    GREET = "!!!! Welcome to MYTLS !!!!"
    GREET_LEN = len(GREET)

    # Chosen option style
    CHOSEN_STYLE = curses.color_pair(1)

    h, w = stdscr.getmaxyx() # Height and width of the terminal

    # ===========
    # CREATE PADS
    # ===========
    welcome_pad = curses.newpad(3, w+GREET_LEN)
    options_pad = curses.newpad(10, w)

    choices = [Choices(x, f"Choice{x}", x == 1) for x in range(1, 11)] # Test choices list

    pad_counter = 0

    stdscr.nodelay(True)

    while True:
        welcome_pad.clear()
        options_pad.clear()

        #* Text moving logic
        welcome_pad.addstr(0, pad_counter, GREET)
        if pad_counter > w-GREET_LEN:
            welcome_pad.addstr(0, 0, GREET[-(pad_counter+GREET_LEN-w):])

        for i, choice in enumerate(choices):
            attr = CHOSEN_STYLE if choice.marked else curses.A_NORMAL
            options_pad.addstr(i, 0, choice.text, attr)

        pad_counter = (pad_counter+1) % w

        welcome_pad.refresh(0, 0, 1, 0, 1, w-1)
        options_pad.refresh(0, 0, 3, 0, 2+len(choices), w-1)

        try:
            key = stdscr.getkey()
        except:
            key = None

        if key == "q":
            break

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