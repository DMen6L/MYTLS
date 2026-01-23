#!/usr/bin/env python3
import argparse
import curses

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
    stdscr.clear()

    stdscr.addstr("Welcome")

    stdscr.refresh()
    stdscr.getch()

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