#include "postgrsql.hpp"

#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

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
