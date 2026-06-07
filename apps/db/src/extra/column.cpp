#include "column.hpp"

#include <stdexcept>

std::string to_sql(const SqlType &type) {
  switch (type) {
  case SqlType::Integer:
    return "INTEGER";

  case SqlType::Text:
    return "TEXT";

  case SqlType::Boolean:
    return "BOOLEAN";

  case SqlType::Float:
    return "FLOAT";

  case SqlType::Timestamp:
    return "TIMESTAMP";
  }

  throw std::runtime_error("no such sql type!");
}
