import psycopg2
from psycopg2.extras import RealDictCursor
import os
from datetime import datetime
from dotenv import load_dotenv

from database import Task
from TypeDefaults import TaskType, Status

load_dotenv()

conn = psycopg2.connect(
    database=os.getenv('DB_NAME'),
    user=os.getenv('DB_USER'),
    password=os.getenv('DB_PASSWORD'),
    port=os.getenv('DB_PORT'),
    cursor_factory=RealDictCursor
)

cur = conn.cursor()

cur.execute("SELECT * FROM tasks")

tasks = [Task(**task) for task in cur.fetchall()]
del_task_ids = []

for task in tasks:
    if task.task_type == TaskType.Daily:
        cur.execute(
            """
            UPDATE tasks SET status = %s WHERE id = %s
            """,
            (Status.NotDone.value, task.id)
        )
    elif task.task_type == TaskType.Once:
        if task.due_date and task.due_date <= datetime.now():
            cur.execute(
                """
                DELETE FROM tasks WHERE id = %s
                """,
                (task.id,)
            )

conn.commit()
cur.close()
conn.close()