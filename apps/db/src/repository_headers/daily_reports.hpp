#ifndef DAILY_REPORTS
#define DAILY_REPORTS

#include "db.hpp"

#include <chrono>
#include <sstream>
#include <string>
#include <vector>

struct DailyReportsTable {
  static constexpr const char *tablename = "daily_reports";

  static std::vector<Column> columns() {
    return {
        Column{.name = "id",
               .type = SqlType::Serial,
               .is_primary_key = true,
               .insertable = false},
        Column{.name = "report_date", .type = SqlType::Date, .unique = true},
        Column{.name = "created_at",
               .type = SqlType::Timestamptz,
               .insertable = false,
               .server_default = "NOW()"},
    };
  }
};

struct DailyReports : DailyReportsTable {
  int id;
  std::chrono::year_month_day report_date;

  std::string get_date() const {
    std::ostringstream ss;

    ss << int(report_date.year()) << "-" << unsigned(report_date.month()) << "-"
       << unsigned(report_date.day());

    return ss.str();
  }

  std::vector<std::string> values() const {

    return {
        std::to_string(id),
        "'" + this->get_date() + "'",
    };
  }
};

struct NewDailyReports : DailyReportsTable {
  std::chrono::year_month_day report_date;

  std::vector<std::string> values() const {
    std::ostringstream ss;

    ss << "'" << int(report_date.year()) << "-" << unsigned(report_date.month())
       << "-" << unsigned(report_date.day()) << "'";

    return {
        ss.str(),
    };
  }
};

DailyReports report_from_row(const pqxx::result::reference &row);

#endif // !DAILY_REPORTS
