import os
import psycopg2
from psycopg2.extras import RealDictCursor
from dotenv import load_dotenv
from dataclasses import dataclass
from typing import List, Optional
from datetime import datetime

from TypeDefaults import Status, TaskType

load_dotenv()

def get_db_connection():
    """
    Establish a connection to the PostgreSQL database.
    """
    conn = psycopg2.connect(
        database=os.getenv('DB_NAME'),
        user=os.getenv('DB_USER'),
        password=os.getenv('DB_PASSWORD'),
        port=os.getenv('DB_PORT'),
        cursor_factory=RealDictCursor
    )
    return conn

@dataclass
class Task():
    id: int
    task_name: str = ""
    task_details: str = ""
    task_type: TaskType = TaskType.Once
    status: Status = Status.NotDone
    due_date: Optional[datetime] = None
    streak: int = 0

    def __post_init__(self):
        if isinstance(self.task_type, str):
            self.task_type = TaskType(self.task_type)
        if isinstance(self.status, str):
            self.status = Status(self.status)
        if isinstance(self.due_date, str) and self.due_date:
            self.due_date = datetime.fromisoformat(self.due_date)

    def get_task_info(self) -> List[str]:
        return [
            str(self.id),
            self.task_name, 
            self.task_type.value,
            self.task_details,
            self.status.value,
            self.due_date.strftime("%Y-%m-%d %H:%M") if self.due_date else "",
            str(self.streak)
        ]

    def get_sql_values(self) -> list:
        return [
            self.id,
            self.task_name,
            self.task_details,
            self.task_type.value,
            self.status.value,
            self.due_date,
            self.streak
        ]

class TasksManager():
    def __init__(self):
        """
        Initialize the TasksManager.
        """
        self.conn = get_db_connection()
        self.cur = self.conn.cursor()

    def add(self, task: Task) -> None:
        """
        Add a new task to the database.
        """
        self.cur.execute(
            """
            INSERT INTO tasks (task_name, task_details, task_type, status, due_date, streak)
            VALUES (%s, %s, %s, %s, %s, %s)
            """,
            tuple(task.get_sql_values()[1:])
        )
        self.conn.commit()
    
    def get_tasks(self) -> List[Task]:
        """
        Retrieve all tasks from the database.
        """
        self.cur.execute(
            """
            SELECT * FROM tasks
            """
        )
        return [Task(**task) for task in self.cur.fetchall()]
    
    def update(self, task: Task) -> None:
        """
        Update a task in the database.
        """
        self.cur.execute(
            "UPDATE tasks SET task_name = %s, task_details = %s, task_type = %s, status = %s, due_date = %s, streak = %s WHERE id = %s",
            (*task.get_sql_values()[1:], task.id)
        )
        self.conn.commit()

    def select(self, id:int) -> Task:
        self.cur.execute(
            """
                SELECT * FROM tasks WHERE id = %s
            """,
            (id,)
        )
        
        row = self.cur.fetchone()

        if row is None:
            return None
        return Task(**row)
    
    def delete(self, del_task: Task) -> bool:
        """
        Delete a task from the database.
        """
        # if del_task.task_type == TaskType.Daily:
        #     return False

        self.cur.execute(
            """
            DELETE FROM tasks WHERE id = %s
            """,
            (del_task.id,)
        )
        self.conn.commit()
        return True
    
    def close_conn(self) -> None:
        """
        Close the database connection.
        """
        self.cur.close()
        self.conn.close()