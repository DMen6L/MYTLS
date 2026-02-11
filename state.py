from dataclasses import dataclass, field
from typing import List, Dict

@dataclass
class AppState():
    state: str = "root"
    running: bool = True
    input_text: str = ""
    todos: List[Dict[str, str]] = field(default_factory=list)