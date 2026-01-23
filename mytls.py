#!/usr/bin/env python3
import argparse

# Test Greeting
def greet(args):
    print(f"Hello, {args.name}")

def main():
    # Main parser and CLI tool
    parser = argparse.ArgumentParser(
        prog="mytls",
        description="Personal CLI tool"
    )

    """
    Subparsers

    greet - test stage
    """
    subparsers = parser.add_subparsers(dest="command")

    greet_parser = subparsers.add_parser("greet", help="Greet user")
    greet_parser.add_argument("-n", "--name", required=True)
    greet_parser.set_defaults(func=greet)

    args = parser.parse_args() # Initialising parser

    if hasattr(args, "func"):
        args.func(args)
    else:
        parser.print_help()
    

if __name__ == "__main__":
    main()