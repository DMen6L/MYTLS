#include "task.hpp"
#include <string>

std::string task_type_to_string(const TaskType &type) {
  switch (type) {
  case TaskType::ONCE:
    return "ONCE";
  case TaskType::DAILY:
    return "DAILY";
  case TaskType::WEEKLY:
    return "WEEKLY";
  case TaskType::MONTHLY:
    return "MONTHLY";
  case TaskType::LONGTERM:
    return "LONGTERM";
  }

  return "ONCE";
}

TaskType task_type_from_string(const std::string &type_str) {
  if (type_str == "ONCE")
    return TaskType::ONCE;
  if (type_str == "DAILY")
    return TaskType::DAILY;
  if (type_str == "WEEKLY")
    return TaskType::WEEKLY;
  if (type_str == "MONTHLY")
    return TaskType::MONTHLY;
  if (type_str == "LONGTERM")
    return TaskType::LONGTERM;

  return TaskType::ONCE;
}

Task task_from_row(const pqxx::result::reference &row) {
  Task task;

  task.id = row["id"].as<int>();
  task.name = row["name"].as<std::string>();
  task.type = task_type_from_string(row["type"].as<std::string>());

  if (!row["deadline"].is_null())
    task.deadline.set(task.type, row["deadline"].as<std::string>());

  task.done = row["done"].as<bool>();
  task.total = row["total"].as<int>();
  task.progress = row["progress"].as<int>();

  return task;
}

int task_type_rank(const TaskType &type) {
  switch (type) {
  case TaskType::LONGTERM:
    return 0;
  case TaskType::DAILY:
    return 1;
  case TaskType::WEEKLY:
    return 2;
  case TaskType::MONTHLY:
    return 3;
  case TaskType::ONCE:
    return 4;
  }

  return 9;
}
