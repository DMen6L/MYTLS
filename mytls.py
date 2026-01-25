#!/usr/bin/env python3
import argparse
import curses
from tui import run_tui

# =================
# STANDARD COMMANDS
# =================
def greet(args):
    print(f"Hello, {args.name}")

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
        curses.wrapper(run_tui)
    elif hasattr(args, "func"):
        args.func(args)
    else:
        parser.print_help()
    

if __name__ == "__main__":
    main()