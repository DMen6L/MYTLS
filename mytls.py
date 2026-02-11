import typer
from tui import run_tui
from state import AppState

app = typer.Typer()
app_state = AppState()

@app.command(short_help="test")
def tui():
    app_state.state = "run tui"

    run_tui(app_state)

@app.command(short_help="test")
def test():
    typer.echo("test")

if __name__ == "__main__":
    app()