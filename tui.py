from rich.console import Console
from rich.live import Live
from rich.panel import Panel

from ui import render_layout
from state import AppState
from database import TasksManager, Task
from TypeDefaults import TaskType, Status

import readchar
from readchar import key
import threading
import time
from typing import List, Dict
from datetime import datetime

VALID_TYPES = {e.value for e in TaskType}
VALID_STATUSES = {e.value for e in Status}

def validate_enums(fields: Dict[str, str]) -> bool:
    """Return True if task_type and status values (when present) are valid enum members."""
    if "task_type" in fields and fields["task_type"] not in VALID_TYPES:
        return False
    if "status" in fields and fields["status"] not in VALID_STATUSES:
        return False
    return True

def parse_due_date(date_str: str):
    """Parse a date string into a datetime object. Returns None if parsing fails."""
    formats = ["%Y-%m-%d %H:%M", "%Y-%m-%d", "%d.%m.%Y"]
    for fmt in formats:
        try:
            return datetime.strptime(date_str, fmt)
        except ValueError:
            continue
    return None

console = Console()
tasks_manager = TasksManager()

def parse_flags(comps: List[str]) -> Dict[str, str]:
    """Parse flag-based input like: -n Task name -d Details -t Once -s Not Done"""
    flags = {"-n": "task_name", "-d": "task_details", "-t": "task_type", "-s": "status", "-due": "due_date"}
    result = {}
    current_flag = None
    current_value = []

    for token in " ".join(comps).split():
        if token in flags:
            if current_flag:
                result[flags[current_flag]] = " ".join(current_value)
            current_flag = token
            current_value = []
        else:
            current_value.append(token)

    if current_flag:
        result[flags[current_flag]] = " ".join(current_value)

    return result

def tui_input(app: AppState):
    """
    Handle user input in the TUI.
    """

    while app.running:
        k = readchar.readkey()

        # Check for commands
        if k == key.ENTER:
            # If no arguments, return
            if not app.input_text.strip():
                app.input_text = ""
                continue
            
            # Break down the command
            command, *comps = app.input_text.strip().split()

            if not command:
                app.input_text = ""
                continue

            # Exit
            if command == "\\q":
                app.state = "root"
                app.running = False
                return

            # Add a task
            if command == "\\add":
                if len(comps) < 2:
                    app.input_text = ""
                    continue
                fields = parse_flags(comps)
                if not validate_enums(fields):
                    app.input_text = ""
                    continue
                new_task = Task(
                    id=len(app.curr_tasks)+1,
                    task_name=fields.get("task_name", "Untitled"),
                    task_details=fields.get("task_details", "None"),
                    task_type=TaskType(fields.get("task_type", "Once")),
                    status=Status(fields.get("status", "Not Done")),
                    due_date=parse_due_date(fields["due_date"]) if "due_date" in fields else None,
                )

                tasks_manager.add(new_task)
                app.curr_tasks.append(new_task)

            # Delete a task
            if command == "\\del":
                del_task = tasks_manager.select(int(comps[0]))

                if del_task is None:
                    app.input_text = ""
                    continue

                if tasks_manager.delete(del_task):
                    app.curr_tasks.remove(del_task)
            
            # Update a task
            if command == "\\update":
                task_id = int(comps[0])
                existing = tasks_manager.select(task_id)
                if existing is None:
                    app.input_text = ""
                    continue

                fields = parse_flags(comps[1:])
                if not validate_enums(fields):
                    app.input_text = ""
                    continue
                if "task_name" in fields:
                    existing.task_name = fields["task_name"]
                if "task_details" in fields:
                    existing.task_details = fields["task_details"]
                if "task_type" in fields:
                    existing.task_type = TaskType(fields["task_type"])
                if "status" in fields:
                    existing.status = Status(fields["status"])
                if "due_date" in fields:
                    existing.due_date = parse_due_date(fields["due_date"])
                
                tasks_manager.update(existing)
                
                for i, t in enumerate(app.curr_tasks):
                    if t.id == task_id:
                        app.curr_tasks[i] = existing
                        break
            
            app.input_text = ""

        elif k == key.BACKSPACE:
            app.input_text = app.input_text[:-1]
        elif len(k) == 1 and k.isprintable():
            app.input_text += k

def run_tui(app: AppState):
    """
    Run the TUI.
    """

    if app.state != "run tui":
        return None

    app.curr_tasks = tasks_manager.get_tasks()
    
    threading.Thread(target=tui_input, args=(app,), daemon=True).start()
    
    with Live(
        render_layout(app),
        console=console,
        screen=True,
        refresh_per_second=10
    ) as live:
        while app.running:
            live.update(render_layout(app))
            time.sleep(0.05)