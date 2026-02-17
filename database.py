import os
import psycopg2
from psycopg2.extras import RealDictCursor
from dotenv import load_dotenv
from dataclasses import dataclass

load_dotenv()

def get_db_connection():
    """Establish a connection to the PostgreSQL database."""
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
    task_type: str = ""
    status: str = ""

    # def __init__(
    #     self,
    #     id: int,
    #     task_name: str,
    #     task_details: str,
    #     task_type: str,
    #     status: str
    # ):
    #     self.id = id
    #     self.task_name = task_name
    #     self.task_details = task_details
    #     self.task_type = task_type
    #     self.status = status

    def get_task_info(self):
        return [
            str(self.id),
            self.task_name, 
            self.task_details,
            self.task_type,
            self.status
        ]

class TasksManager():
    def __init__(self):
        self.conn = get_db_connection()
        self.cur = self.conn.cursor()

    def add(self, task: Task):
        self.cur.execute(
            """
            INSERT INTO tasks (task_name, task_details, task_type, status)
            VALUES (%s, %s, %s, %s)
            """,
            (task.task_name, task.task_details, task.task_type, task.status)
        )
        self.conn.commit()
    
    def get_tasks(self):
        self.cur.execute(
            """
            SELECT * FROM tasks
            """
        )
        return [Task(**task) for task in self.cur.fetchall()]
    
    def update(self, task: Task):
        self.cur.execute(
            "UPDATE tasks SET task_name = %s, task_details = %s, task_type = %s, status = %s WHERE id = %s",
            (task.task_name, task.task_details, task.task_type, task.status, task.id)
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
        if del_task.task_type == "Daily":
            return False

        self.cur.execute(
            """
            DELETE FROM tasks WHERE id = %s
            """,
            (del_task.id,)
        )
        self.conn.commit()
        return True
    
    def close_conn(self):
        self.cur.close()
        self.conn.close()