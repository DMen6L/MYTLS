import typer
from tui import run_tui

app = typer.Typer()

@app.command(short_help="test")
def tui():
    run_tui()

@app.command(short_help="test")
def test():
    typer.echo("test")

if __name__ == "__main__":
    app()