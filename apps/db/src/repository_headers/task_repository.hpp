#ifndef TASK_REPO
#define TASK_REPO

#include "db.hpp"
#include "task.hpp"

#include <string>
#include <vector>

class TaskRepository {
  pqxx::connection &conn_;
  std::vector<Task> tasks;

public:
  explicit TaskRepository(pqxx::connection &conn);
  void init_table();
};

#endif
