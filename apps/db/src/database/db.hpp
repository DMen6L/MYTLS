#ifndef DB
#define DB

#include <optional>
#include <pqxx/pqxx>
#include <string>

std::string db_init_conn(
    const std::string &conn_file_path = "/home/dmensxl/projects/MYTLS/.env");

enum class SqlType {
  Serial,
  Integer,
  Text,
  Varchar,
  Boolean,
  Float,
  Timestamp,
  Timestamptz,
  Date
};

struct Column {
  std::string name;
  SqlType type;
  std::optional<int> length = std::nullopt;

  bool is_primary_key = false;
  bool auto_increment = false;
  bool unique = false;
  bool nullable = true;
  bool insertable = true;
  std::optional<std::string> foreign_key = std::nullopt;
  std::optional<std::string> server_default = std::nullopt;

  bool is_valid() const;
};

std::string to_sql(const SqlType &type);
std::string to_sql(const Column &col);

struct Migration {
  int version;
  std::string sql;
};

#endif
