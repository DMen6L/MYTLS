#include "db.hpp"
#include "transactor.hpp"

#include <string>
#include <vector>

int main() {
  std::string conn_str = db_init_conn();
  Transactor trans(conn_str);
  std::vector<Migration> migrations = {
      {1, R"(
    CREATE TABLE IF NOT EXISTS schema_migrations (
    version INTEGER PRIMARY KEY
    );
    )"},
      {2, R"(ALTER TABLE tasks 
    ALTER COLUMN deadline TYPE VARCHAR(100);)"},
      {3, R"(ALTER TABLE tasks
      ADD COLUMN total INTEGER DEFAULT 1;)"},
      {4, R"(ALTER TABLE tasks
      ADD COLUMN progress INTEGER DEFAULT 0;)"},
      {5, R"(ALTER TABLE tasks
      RENAME COLUMN progres TO progress;)"},
  };

  for (auto &migration : migrations) {
    trans.migrate(migration);
  }
  return 0;
}
