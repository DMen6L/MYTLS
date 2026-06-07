#ifndef TRANSACTOR
#define TRANSACTOR

#include "db.hpp"
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
      ss << cols[i].name << " " << to_sql(cols[i].type);

      if (i + 1 < cols.size())
        ss << ", ";
    }

    ss << ");";

    pqxx::work tx(conn_);
    tx.exec(ss.str());
    tx.commit();
  };
};

#endif
