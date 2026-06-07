#ifndef TASK
#define TASK

#include <string>
#include <vector>

#include "postgrsql.hpp"

struct Task {
  int id;
  std::string name;

  static constexpr const char *tablename = "tasks";
  static std::vector<Column> columns() {
    return {{"id", SqlType::Integer}, {"name", SqlType::Text}};
  };
};

#endif // !TASK
