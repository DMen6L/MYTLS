from dataclasses import dataclass, field
from typing import List
from database import Task

@dataclass
class AppState():
    state: str = "root"
    running: bool = True
    input_text: str = ""
    curr_tasks: List[Task] = field(default_factory=list)