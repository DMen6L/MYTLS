#include "todo.hpp"
#include "db.hpp"
#include "task.hpp"
#include "transactor.hpp"

#include <iostream>
#include <pqxx/pqxx>
#include <stdexcept>

bool add_task(todo_state &state, const std::string &new_task) {
  state.tasks.push_back(new_task);
  return true;
}

void todo_render(todo_state &state) {
  std::string conn_str = db_init_conn();
  Transactor db_trans(conn_str);

  db_trans.test_conn();
  db_trans.init_table<Task>();
}
