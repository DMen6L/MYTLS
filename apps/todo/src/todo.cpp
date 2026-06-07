#include "todo.hpp"
#include "db.hpp"
#include "task_repository.hpp"

#include <iostream>
#include <pqxx/pqxx>
#include <stdexcept>

bool add_task(todo_state &state, const std::string &new_task) {
  state.tasks.push_back(new_task);
  return true;
}

void todo_render(todo_state &state) {
  pqxx::connection conn = db_init_conn();

  if (!conn.is_open())
    throw std::runtime_error("Failed to connect");

  TaskRepository task_repo(conn);
  task_repo.init_table();
}
