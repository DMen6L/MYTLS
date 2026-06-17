#ifndef TASK
#define TASK

#include <array>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include "db.hpp"
#include "postgrsql.hpp"

enum class TaskType { ONCE, DAILY, WEEKLY, LONGTERM };
constexpr std::array<TaskType, 4> AllTaskTypes{
    TaskType::ONCE, TaskType::DAILY, TaskType::WEEKLY, TaskType::LONGTERM};

std::string task_type_to_string(const TaskType &type);
TaskType task_type_from_string(const std::string &type_str);

struct TaskTable {
  static constexpr const char *tablename = "tasks";

  static std::vector<Column> columns() {
    return {Column{.name = "id",
                   .type = SqlType::Serial,
                   .is_primary_key = true,
                   .insertable = false},
            Column{.name = "name", .type = SqlType::Varchar, .length = 255},
            Column{.name = "done",
                   .type = SqlType::Boolean,
                   .server_default = "FALSE"},
            Column{.name = "deadline", .type = SqlType::Timestamptz},
            Column{.name = "type",
                   .type = SqlType::Varchar,
                   .length = 100,
                   .server_default = "'ONCE'"}};
  };
};

struct Task : TaskTable {
  int id;
  std::string name;
  bool done;
  std::optional<std::string> deadline;
  TaskType type;

  std::vector<std::string> values() const {
    return {std::to_string(id), "'" + name + "'", done ? "TRUE" : "FALSE",
            deadline ? "'" + *deadline + "'" : "NULL",
            "'" + task_type_to_string(type) + "'"};
  }
};

struct NewTask : TaskTable {
  std::string name;
  bool done = false;
  std::optional<std::string> deadline = std::nullopt;
  TaskType type;

  std::vector<std::string> values() const {
    return {"'" + name + "'", done ? "TRUE" : "FALSE",
            deadline ? "'" + *deadline + "'" : "NULL",
            "'" + task_type_to_string(type) + "'"};
  }
};

struct UpdateTask : TaskTable {
  int id;
  std::optional<std::string> name = std::nullopt;
  std::optional<bool> done = std::nullopt;
  std::optional<std::string> deadline = std::nullopt;
  std::optional<TaskType> type = std::nullopt;

  std::vector<std::string> values() const {
    std::string name_str = "";
    std::string done_str = "";
    std::string deadline_str = "";
    std::string type_str = "";

    if (name)
      name_str = "'" + *name + "'";
    if (done) {
      done_str = *done ? "TRUE" : "FALSE";
    }
    if (deadline)
      deadline_str = "'" + *deadline + "'";
    if (type)
      type_str = "'" + task_type_to_string(*type) + "'";

    return {std::to_string(id), name_str, done_str, deadline_str, type_str};
  }
};

Task task_from_row(const pqxx::result::reference &row);

#endif // !TASK
