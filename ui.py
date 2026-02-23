from rich.panel import Panel
from rich.table import Table
from rich.layout import Layout
from rich.align import Align
from typing import List

from state import AppState
from database import Task

def render_layout(app: AppState):
    layout = Layout()

    # Table layout
    table = Table(title="Todos")
    table.add_column("Id")
    table.add_column("Task")
    table.add_column("Type")
    table.add_column("Details", width=50)
    table.add_column("Status")
    table.add_column("Due Date")
    table.add_column("Streak")

    for task in app.curr_tasks:
        table.add_row(
            *task.get_task_info()
        )

    # Basic layout
    layout.split_column(
        Layout(
            name="header", 
            size=3
            ),
        Layout(
            name="body"
        ),
        Layout(
            name="input",
            size=3
        ),
        Layout(
            name="footer",
            size=3
        ),
    )

    # Header layout
    layout["header"].update(
        Layout(
            Panel(
                Align.center("My app"),
                style="bold cyan"
            )
        )
    )

    # Body layout
    layout["body"].update(
        Align.center(table)
    )

    layout["input"].update(
        Layout(
            Panel(
                app.input_text,
                style="bold cyan"
            )
        )
    )

    # Footer layout
    layout["footer"].update(
        Layout(Panel("CTRL+C to exit", style="bold red"))
    )

    return layout