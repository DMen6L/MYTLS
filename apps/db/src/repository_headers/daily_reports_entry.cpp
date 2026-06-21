#include "daily_reports_entry.hpp"
#include <string>

DailyReportsEntry report_entry_from_row(const pqxx::result::reference &row) {
  DailyReportsEntry report_entry;

  report_entry.reports_id = row["reports_id"].as<int>();
  report_entry.task_name = row["task_name"].as<std::string>();
  report_entry.progress = row["progress"].as<int>();
  report_entry.total = row["total"].as<int>();
  report_entry.was_completed = row["was_completed"].as<bool>();

  return report_entry;
}
