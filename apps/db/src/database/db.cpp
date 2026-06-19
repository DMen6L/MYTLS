#include "db.hpp"
#include "env.hpp"

#include <format>
#include <pqxx/pqxx>
#include <string>
#include <unordered_map>

std::string db_init_conn(const std::string &conn_file_path) {
  std::unordered_map<std::string, std::string> env = read_env(conn_file_path);
  // TODO: save conn_str as URI
  return std::format("host={} port={} dbname={} user={} password={}",
                     env["DB_HOST"], env["DB_PORT"], env["DB_NAME"],
                     env["DB_USER"], env["DB_PASSWORD"]);
}

bool Column::is_valid() const {
  if (type == SqlType::Varchar && !length)
    return false;
  if (type != SqlType::Varchar && length)
    return false;

  return true;
}

std::string to_sql(const SqlType &type) {
  switch (type) {
  case SqlType::Serial:
    return "SERIAL";

  case SqlType::Integer:
    return "INTEGER";

  case SqlType::Text:
    return "TEXT";

  case SqlType::Varchar:
    return "VARCHAR";

  case SqlType::Boolean:
    return "BOOLEAN";

  case SqlType::Float:
    return "FLOAT";

  case SqlType::Timestamp:
    return "TIMESTAMP";

  case SqlType::Timestamptz:
    return "TIMESTAMPTZ";
  }

  throw std::runtime_error("no such sql type!");
}

std::string to_sql(const Column &col) {
  std::stringstream ss;

  ss << col.name << ' ' << to_sql(col.type);

  if (col.is_valid() && col.length)
    ss << '(' << *col.length << ')';

  if (col.unique)
    ss << " UNIQUE";

  if (col.is_primary_key)
    ss << " PRIMARY KEY";

  if (!col.nullable)
    ss << " NOT NULL";

  if (col.server_default)
    ss << " DEFAULT " << *col.server_default;

  return ss.str();
}
