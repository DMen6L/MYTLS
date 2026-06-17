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
  if (type_str == "LONGTERM")
    return TaskType::LONGTERM;

  return TaskType::ONCE;
}

Task task_from_row(const pqxx::result::reference &row) {
  Task task;

  task.id = row["id"].as<int>();
  task.name = row["name"].as<std::string>();
  // deadline is optional
  if (row["deadline"].is_null())
    task.deadline = std::nullopt;
  else
    task.deadline = row["deadline"].as<std::string>();
  task.type = task_type_from_string(row["type"].as<std::string>());
  task.done = row["done"].as<bool>();

  return task;
}
