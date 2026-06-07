#include <cstddef>
#include <sstream>
#include <string>

#include "column.hpp"
#include "db.hpp"
#include "task.hpp"
#include "task_repository.hpp"

TaskRepository::TaskRepository(pqxx::connection &conn) : conn_(conn) {}

void TaskRepository::init_table() {
  std::stringstream ss;
  ss << "CREATE TABLE IF NOT EXISTS " << Task::tablename << " (";

  std::vector<Column> cols = Task::columns();

  for (size_t i = 0; i < cols.size(); i++) {
    ss << cols[i].name << ' ' << to_sql(cols[i].type);

    if (i + 1 < cols.size())
      ss << ", ";
  }

  ss << ");";

  std::string query = ss.str();

  pqxx::work tx(conn_);

  tx.exec(query);
  tx.commit();
}
