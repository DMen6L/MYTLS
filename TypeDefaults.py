from enum import Enum

class Status(Enum):
    default = "Not Done"
    NotDone = "Not Done"
    Progress = "Progress"
    Done = "Done"

class TaskType(Enum):
    default = "Once"
    Once = "Once"
    Daily = "Daily"
    Homework = "Homework"
    Weekly = "Weekly"
    Monthly = "Monthly"
    Yearly = "Yearly"
