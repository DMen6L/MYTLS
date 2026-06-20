#ifndef TASK
#define TASK

#include <array>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "db.hpp"

enum class TaskType { ONCE, DAILY, WEEKLY, MONTHLY, LONGTERM };
constexpr std::array<TaskType, 5> AllTaskTypes{
    TaskType::ONCE, TaskType::DAILY, TaskType::WEEKLY, TaskType::MONTHLY,
    TaskType::LONGTERM};

std::string task_type_to_string(const TaskType &type);
TaskType task_type_from_string(const std::string &type_str);

int task_type_rank(const TaskType &type);

struct TaskSchedule {
  struct Once {
    std::optional<std::chrono::system_clock::time_point> tp;
  };
  struct Daily {
    std::optional<std::chrono::minutes> time;
  };
  struct Weekly {
    std::optional<int> weekday;
  };
  struct Monthly {
    std::optional<int> day;
  };

  using Deadline = std::variant<std::monostate, Once, Daily, Weekly, Monthly>;
  Deadline deadline = std::monostate{};
  bool is_set() const {
    return std::visit(
        [](const auto &v) {
          using T = std::decay_t<decltype(v)>;

          if constexpr (std::is_same_v<T, std::monostate>)
            return false;
          else if constexpr (std::is_same_v<T, Once>)
            return v.tp.has_value();
          else if constexpr (std::is_same_v<T, Daily>)
            return v.time.has_value();
          else if constexpr (std::is_same_v<T, Weekly>)
            return v.weekday.has_value();
          else if constexpr (std::is_same_v<T, Monthly>)
            return v.day.has_value();
        },
        deadline);
  }

  bool operator<(const TaskSchedule &other) const {
    return std::visit(
        [](const auto &a, const auto &b) -> bool {
          using A = std::decay_t<decltype(a)>;
          using B = std::decay_t<decltype(b)>;

          if constexpr (!std::is_same_v<A, B>) {
            return false; // should never happen
          } else if constexpr (std::is_same_v<A, Once>) {
            return *a.tp < *b.tp;
          } else if constexpr (std::is_same_v<A, Daily>) {
            return *a.time < *b.time;
          } else if constexpr (std::is_same_v<A, Weekly>) {
            return *a.weekday < *b.weekday;
          } else if constexpr (std::is_same_v<A, Monthly>) {
            return *a.day < *b.day;
          } else {
            return false;
          }
        },
        deadline, other.deadline);
  }

  void set(const TaskType &type, const std::string &val) {
    if (val.empty())
      return;

    std::istringstream ss(val);

    switch (type) {
    case TaskType::ONCE: {
      std::tm tm{};
      ss >> std::get_time(&tm, "%Y-%m-%d");
      std::time_t t = std::mktime(&tm);
      deadline = Once{std::chrono::system_clock::from_time_t(t)};
      break;
    }
    case TaskType::DAILY: {
      int h, m;
      char c;
      ss >> h >> c >> m;
      deadline = Daily{std::chrono::minutes(h * 60 + m)};
      break;
    }
    case TaskType::WEEKLY: {
      int w = std::stoi(val);
      deadline = Weekly{w};
      break;
    }
    case TaskType::MONTHLY: {
      int d = std::stoi(val);
      deadline = Monthly{d};
      break;
    }
    case TaskType::LONGTERM: {
      break;
    }
    }
  }
  // TODO: finish to string definitions
  std::string to_string() const {
    if (!this->is_set())
      return "";
    return std::visit(
        [](auto &v) -> std::string {
          using T = std::decay_t<decltype(v)>;

          if constexpr (std::is_same_v<T, std::monostate>) {
            return "";
          }

          else if constexpr (std::is_same_v<T, Once>) {
            if (!v.tp.has_value())
              return "";
            std::time_t t = std::chrono::system_clock::to_time_t(*v.tp);
            std::tm *tm = std::localtime(&t);

            std::ostringstream ss;
            ss << std::put_time(tm, "%Y-%m-%d");
            return ss.str();
          }

          else if constexpr (std::is_same_v<T, Daily>) {
            if (!v.time.has_value())
              return "";
            int total = (*v.time).count();
            int h = total / 60;
            int m = total % 60;

            std::ostringstream ss;
            ss << h << ":" << (m < 10 ? "0" : "") << m;
            return ss.str();
          }

          else if constexpr (std::is_same_v<T, Weekly>) {
            if (!v.weekday.has_value())
              return "";
            return std::to_string(*v.weekday);
          }

          else if constexpr (std::is_same_v<T, Monthly>) {
            if (!v.day.has_value())
              return "";
            return std::to_string(*v.day);
          }

          return "";
        },
        deadline);
  }
};

struct TaskTable {
  static constexpr const char *tablename = "tasks";

  static std::vector<Column> columns() {
    return {
        Column{.name = "id",
               .type = SqlType::Serial,
               .is_primary_key = true,
               .insertable = false},
        Column{.name = "name", .type = SqlType::Varchar, .length = 255},
        Column{.name = "done",
               .type = SqlType::Boolean,
               .server_default = "FALSE"},
        Column{.name = "deadline", .type = SqlType::Varchar, .length = 100},
        Column{.name = "type",
               .type = SqlType::Varchar,
               .length = 100,
               .server_default = "'ONCE'"},
        Column{
            .name = "total", .type = SqlType::Integer, .server_default = "1"},
        Column{.name = "progress",
               .type = SqlType::Integer,
               .server_default = "0"},
    };
  };
};

struct Task : TaskTable {
  int id;
  std::string name;
  bool done;
  TaskType type;
  int total;
  int progress;

  // deadline definitions
  TaskSchedule deadline;

  float get_progress() const {
    if (this->total < 1)
      return 0;
    return static_cast<float>(this->progress) / this->total;
  }

  std::vector<std::string> values() const {
    return {
        std::to_string(id),
        "'" + name + "'",
        done ? "TRUE" : "FALSE",
        !deadline.to_string().empty() ? "'" + deadline.to_string() + "'"
                                      : "NULL",
        "'" + task_type_to_string(type) + "'",
        std::to_string(total),
        std::to_string(progress),
    };
  }
};

struct NewTask : TaskTable {
  std::string name;
  bool done = false;
  TaskType type;
  int total;
  int progress;

  TaskSchedule deadline;

  std::vector<std::string> values() const {
    return {
        "'" + name + "'",
        done ? "TRUE" : "FALSE",
        !deadline.to_string().empty() ? "'" + deadline.to_string() + "'"
                                      : "NULL",
        "'" + task_type_to_string(type) + "'",
        std::to_string(total),
        std::to_string(progress),
    };
  }
};

struct UpdateTask : TaskTable {
  int id;
  std::optional<std::string> name = std::nullopt;
  std::optional<bool> done = std::nullopt;
  TaskSchedule deadline;
  std::optional<TaskType> type = std::nullopt;
  std::optional<int> total = std::nullopt;
  std::optional<int> progress = std::nullopt;

  std::vector<std::string> values() const {
    std::string name_str = "";
    std::string done_str = "";
    std::string deadline_str = "";
    std::string type_str = "";
    std::string total_str = "";
    std::string progress_str = "";

    if (name)
      name_str = "'" + *name + "'";
    if (done) {
      done_str = *done ? "TRUE" : "FALSE";
    }

    deadline_str = !deadline.to_string().empty()
                       ? "'" + deadline.to_string() + "'"
                       : "NULL";

    if (type)
      type_str = "'" + task_type_to_string(*type) + "'";

    if (total)
      total_str = std::to_string(*total);

    if (progress)
      progress_str = std::to_string(*progress);

    return {
        std::to_string(id), name_str,  done_str,     deadline_str,
        type_str,           total_str, progress_str,
    };
  }
};

Task task_from_row(const pqxx::result::reference &row);

#endif // !TASK
