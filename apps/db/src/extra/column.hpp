#ifndef COLUMN
#define COLUMN

#include <string>

enum class SqlType { Integer, Text, Boolean, Float, Timestamp };
std::string to_sql(const SqlType &type);

struct Column {
  std::string name;
  SqlType type;
};

#endif
