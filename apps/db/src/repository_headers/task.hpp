#ifndef TASK
#define TASK

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include "postgrsql.hpp"

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
            Column{.name = "deadline", .type = SqlType::Timestamptz}};
  };
};

struct Task : TaskTable {
  int id;
  std::string name;
  bool done;
  std::optional<std::string> deadline;

  std::vector<std::string> values() const {
    return {std::to_string(id), "'" + name + "'", done ? "TRUE" : "FALSE",
            deadline ? "'" + *deadline + "'" : "NULL"};
  }
};

struct NewTask : TaskTable {
  std::string name;
  bool done = false;
  std::optional<std::string> deadline = std::nullopt;

  std::vector<std::string> values() const {
    return {"'" + name + "'", done ? "TRUE" : "FALSE",
            deadline ? "'" + *deadline + "'" : "NULL"};
  }
};

#endif // !TASK
