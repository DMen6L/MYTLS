from dataclasses import dataclass

@dataclass
class AppState():
    state: str = "root"
    done: bool = False