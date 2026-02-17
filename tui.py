from rich.console import Console
from rich.live import Live
from rich.panel import Panel

from ui import render_layout
from state import AppState
from database import TasksManager, Task

import readchar
from readchar import key
import threading
import time
from typing import List, Dict

console = Console()
tasks_manager = TasksManager()

def parse_flags(comps: List[str]) -> Dict[str, str]:
    """Parse flag-based input like: -n Task name -d Details -t Once -s Not Done"""
    flags = {"-n": "task_name", "-d": "task_details", "-t": "task_type", "-s": "status"}
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

            if len(comps) < 2:
                app.input_text = ""
                continue
            
            # Add a task
            if command == "\\add":
                fields = parse_flags(comps)
                new_task = Task(
                    id=len(app.curr_tasks)+1,
                    task_name=fields.get("task_name", "Untitled"),
                    task_details=fields.get("task_details", "None"),
                    task_type=fields.get("task_type", "Once"),
                    status=fields.get("status", "Not Done"),
                )

                tasks_manager.add(new_task)
                app.curr_tasks.append(new_task)

            # Delete a task
            if command == "\\del":
                del_task = tasks_manager.select(int(comps[1]))

                if del_task is None:
                    app.input_text = ""
                    continue

                if tasks_manager.delete(del_task):
                    app.curr_tasks.remove(del_task)
            
            # Update a task
            if command == "\\update":
                if len(comps) < 2:
                    app.input_text = ""
                    continue   
                task_id = int(comps[1])
                existing = tasks_manager.select(task_id)
                if existing is None:
                    app.input_text = ""
                    continue

                fields = parse_flags(" ".join(comps[2:]))
                existing.task_name = fields.get("task_name", existing.task_name)
                existing.task_details = fields.get("task_details", existing.task_details)
                existing.task_type = fields.get("task_type", existing.task_type)
                existing.status = fields.get("status", existing.status)
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