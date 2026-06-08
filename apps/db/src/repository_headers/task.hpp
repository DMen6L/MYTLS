#ifndef TASK
#define TASK

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
            Column{.name = "done", .type = SqlType::Boolean}};
  };
};

struct Task : TaskTable {
  int id;
  std::string name;
  bool done;

  std::vector<std::string> values() const {
    return {std::to_string(id), "'" + name + "'", done ? "TRUE" : "FALSE"};
  }
};

struct NewTask : TaskTable {
  std::string name;
  bool done = false;

  std::vector<std::string> values() const {
    return {"'" + name + "'", done ? "TRUE" : "FALSE"};
  }
};

#endif // !TASK
