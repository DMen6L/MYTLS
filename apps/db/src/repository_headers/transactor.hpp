#ifndef TRANSACTOR
#define TRANSACTOR

#include "db.hpp"
#include "postgrsql.hpp"

#include <iostream>
#include <sstream>
#include <string>

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

  template <typename T> void insert(const T &obj) {
    auto cols = T::columns();
    auto vals = obj.values();
    int n = cols.size();
    int m = vals.size();

    std::stringstream ss;

    ss << "INSERT INTO " << T::tablename << " (";

    for (size_t i = 0; i < n; i++) {
      if (!cols[i].insertable)
        continue;
      ss << cols[i].name;

      if (i + 1 < n)
        ss << ", ";
    }

    ss << ") VALUES (";

    for (size_t i = 0; i < m; i++) {
      ss << vals[i];

      if (i + 1 < m)
        ss << ", ";
    }

    ss << ");";

    pqxx::work tx(conn_);
    std::cout << ss.str();
    tx.exec(ss.str());
    tx.commit();
  }
};

#endif
