from rich.console import Console
from rich.live import Live
from rich.panel import Panel
from ui import render_layout

from state import AppState

import readchar
from readchar import key
import threading
import time

from database import TasksManager, Task

console = Console()
tasks_manager = TasksManager()

def tui_input(app: AppState):
    while app.running:
        k = readchar.readkey()

        # Check for commands
        if k == key.ENTER:
            # If no arguments, return
            if not app.input_text.strip():
                app.input_text = ""
                continue
            
            # Break down the command
            comps = app.input_text.strip().split()
            if not comps:
                app.input_text = ""
                continue

            if comps[0] == "\\q":
                app.state = "root"
                app.running = False
                return
            
            # Add a task
            if comps[0] == "ADD":
                if len(comps) < 2:
                    pass
                else:
                    new_task = Task(
                        id=len(app.curr_tasks)+1,
                        task_name=comps[1],
                        task_details="None",
                        task_type="Once",
                        status="Not Done"
                    )

                    tasks_manager.add_task(new_task)
                    app.curr_tasks.append(new_task)
            
            app.input_text = ""

        elif k == key.BACKSPACE:
            app.input_text = app.input_text[:-1]
        elif len(k) == 1 and k.isprintable():
            app.input_text += k

def run_tui(app: AppState):
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