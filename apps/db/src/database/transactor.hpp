#ifndef TRANSACTOR
#define TRANSACTOR

#include "db.hpp"

#include <sstream>
#include <string>

// TODO: finish all endpoints
class Transactor {
  pqxx::connection conn_;

public:
  explicit Transactor(const std::string &conn_str) : conn_(conn_str) {};

  bool test_conn() {
    if (!conn_.is_open())
      throw std::runtime_error("Failed to connect");
    else
      return true;
  };

  template <typename Table> void init_table() {
    std::stringstream ss;

    ss << "CREATE TABLE IF NOT EXISTS " << Table::tablename << " (";

    auto cols = Table::columns();

    for (size_t i = 0; i < cols.size(); i++) {
      ss << to_sql(cols[i]);

      if (i + 1 < cols.size())
        ss << ", ";
    }

    ss << ");";

    pqxx::work tx(conn_);
    tx.exec(ss.str());
    tx.commit();
  };

  template <typename Table> pqxx::result select_all() {
    std::stringstream ss;

    ss << "SELECT * FROM " << Table::tablename << ';';

    pqxx::work tx(conn_);
    pqxx::result res = tx.exec(ss.str());
    tx.commit();

    return res;
  }

  template <typename Table>
  pqxx::result select_by_name(const std::string &name, const Column &col) {
    std::stringstream ss;

    ss << "SELECT * FROM " << Table::tablename << " WHERE " << col.name << " = "
       << name << ';';

    pqxx::work tx(conn_);
    pqxx::result res = tx.exec(ss.str());
    tx.commit();

    return res;
  }

  template <typename T> pqxx::result insert(const T &obj) {
    auto cols = T::columns();
    auto vals = obj.values();
    int n = cols.size();
    int m = vals.size();

    std::stringstream ss;
    bool first_insert = false;

    ss << "INSERT INTO " << T::tablename << " (";

    for (size_t i = 0; i < n; i++) {
      if (!cols[i].insertable)
        continue;

      if (i > 0 && first_insert)
        ss << ", ";

      if (!first_insert)
        first_insert = true;

      ss << cols[i].name;
    }

    ss << ") VALUES (";

    for (size_t i = 0; i < m; i++) {
      ss << vals[i];

      if (i + 1 < m)
        ss << ", ";
    }

    ss << ") RETURNING *;";

    pqxx::work tx(conn_);
    pqxx::result res = tx.exec(ss.str());
    tx.commit();

    return res;
  }

  template <typename T> void delete_by_id(int id) {
    std::stringstream ss;

    ss << "DELETE FROM " << T::tablename << " WHERE id = " << id << ';';

    pqxx::work tx(conn_);
    tx.exec(ss.str());
    tx.commit();
  }

  template <typename T> void update(const T &obj) {
    auto cols = T::columns();
    auto vals = obj.values();
    int n = cols.size();
    int m = vals.size();

    std::stringstream ss;
    bool first_update = true;

    ss << "UPDATE " << T::tablename << " SET ";

    for (size_t i = 0; i < n; i++) {
      if (!cols[i].insertable || vals[i].empty())
        continue;

      if (!first_update)
        ss << ", ";

      ss << cols[i].name << " = " << vals[i];
      first_update = false;
    }

    ss << " WHERE id = " << vals[0] << ';';

    pqxx::work tx(conn_);
    tx.exec(ss.str());
    tx.commit();
  }

  void migrate(Migration &migration) {
    pqxx::work tx(conn_);

    auto res = tx.exec("SELECT 1 FROM schema_migrations WHERE version = $1",
                       migration.version);

    if (!res.empty()) {
      tx.commit();
      return;
    }

    tx.exec(migration.sql);
    tx.exec("INSERT INTO schema_migrations(version) VALUES($1)",
            migration.version);
    tx.commit();
  }
};

#endif
