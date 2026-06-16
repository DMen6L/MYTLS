#include "task.hpp"

Task task_from_row(const pqxx::result::reference &row) {
  Task task;

  task.id = row["id"].as<int>();
  task.name = row["name"].as<std::string>();
  // deadline is optional
  if (row["deadline"].is_null())
    task.deadline = std::nullopt;
  else
    task.deadline = row["deadline"].as<std::string>();
  task.type = row["type"].as<std::string>();
  task.done = row["done"].as<bool>();

  return task;
}
