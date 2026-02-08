from rich.console import Console
from rich.live import Live
from rich.panel import Panel
from ui import render_layout

from state import AppState

import threading
import time

console = Console()

def tui_input():
    pass

def run_tui(app: AppState):
    if app.state != "run tui":
        return None
    
    layout = render_layout()
    
    with Live(
        layout,
        console=console,
        screen=True,
        refresh_per_second=5
    ):
        i = 0
        while i < 20:
            layout.update(render_layout())

            time.sleep(1)
            i += 1
    pass