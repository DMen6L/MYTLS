#include "daily_reports.hpp"
#include <chrono>
#include <sstream>
#include <string>

std::chrono::year_month_day parse_date(const std::string &s) {
  int y, m, d;
  char dash1, dash2;

  std::istringstream iss(s);
  iss >> y >> dash1 >> m >> dash2 >> d;

  return std::chrono::year{y} / std::chrono::month{static_cast<unsigned>(m)} /
         std::chrono::day{static_cast<unsigned>(d)};
}

DailyReports report_from_row(const pqxx::result::reference &row) {
  DailyReports report;

  report.id = row["id"].as<int>();

  std::string date_str = row["report_date"].as<std::string>();
  report.report_date = parse_date(date_str);

  return report;
}
