#ifndef DAILY_REPORTS_ENTRY
#define DAILY_REPORTS_ENTRY

#include "db.hpp"
#include <string>

struct DailyReportsEntryTable {
  static constexpr const char *tablename = "daily_reports_entry";

  static std::vector<Column> columns() {
    return {
        Column{.name = "reports_id",
               .type = SqlType::Integer,
               .foreign_key = "daily_reports(id)"},
        Column{.name = "task_name", .type = SqlType::Text},
        Column{.name = "progress", .type = SqlType::Integer},
        Column{.name = "total", .type = SqlType::Integer},
        Column{.name = "was_completed", .type = SqlType::Boolean},
    };
  }
};

struct DailyReportsEntry : DailyReportsEntryTable {
  int reports_id;
  std::string task_name;
  int progress;
  int total;
  bool was_completed;

  std::vector<std::string> values() const {
    return {
        std::to_string(reports_id),       "'" + task_name + "'",
        std::to_string(progress),         std::to_string(total),
        was_completed ? "TRUE" : "FALSE",
    };
  }
};

struct NewDailyReportsEntry : DailyReportsEntryTable {
  int reports_id;
  std::string task_name;
  int progress;
  int total;
  bool was_completed;

  std::vector<std::string> values() const {
    return {
        std::to_string(reports_id),       "'" + task_name + "'",
        std::to_string(progress),         std::to_string(total),
        was_completed ? "TRUE" : "FALSE",
    };
  }
};

DailyReportsEntry report_entry_from_row(const pqxx::result::reference &row);

#endif
