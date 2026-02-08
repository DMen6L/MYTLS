from rich.panel import Panel
from rich.table import Table
from rich.layout import Layout
from rich.align import Align

def render_layout():
    layout = Layout()

    # Table layout
    table = Table(title="Todos")
    table.add_column("Id")
    table.add_column("Task")
    table.add_column("Type")
    table.add_column("Status")

    table.add_row("1", "asdasd", "sadas", ":x:")
    table.add_row(
        "2", 
        "shop", 
        "Daily",
        ":white_heavy_check_mark:"
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

    # Footer layout
    layout["footer"].update(
        Layout(Panel("CTRL+C to exit", style="bold red"))
    )

    return layout