#ifndef COLUMN
#define COLUMN

#include <optional>
#include <string>

enum class SqlType {
  Serial,
  Integer,
  Text,
  Varchar,
  Boolean,
  Float,
  Timestamp
};

struct Column {
  std::string name;
  SqlType type;
  std::optional<int> length = std::nullopt;

  bool is_primary_key = false;
  bool auto_increment = false;
  bool insertable = true;

  bool is_valid() const;
};

std::string to_sql(const SqlType &type);
std::string to_sql(const Column &col);

#endif
