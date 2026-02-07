from rich.panel import Panel
from rich.table import Table
from rich.layout import Layout

def render_layout():
    layout = Layout()

    table = Table(title="Stats")
    table.add_column("Test")
    table.add_column("maybe")

    table.add_row("Step 1")
    table.add_row("Status OK")

    layout.split_column(
        Layout(
            Panel("My App", style="bold cyan"), 
            size=3
            ),
        table,
        Layout(
            Panel("CTRL_C to exit", style="bold red"),
            size=3
        ),
    )

    return layout