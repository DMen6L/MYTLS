from rich.console import Console
from rich.live import Live
from rich.panel import Panel
from ui import render_layout
import time

console = Console()

def run_tui():
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