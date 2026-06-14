#include "todo.hpp"
#include "task.hpp"
#include "transactor.hpp"

#include <ftxui/component/app.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <optional>
#include <pqxx/pqxx>
#include <sstream>
#include <utility>
#include <vector>

ftxui::Component todo_render(Transactor &trans) {
  TodoState state(trans);
  state.db_trans.init_table<TaskTable>();
  state.db_trans.insert<NewTask>(NewTask{
      .name = "new task", .deadline = "2026-12-12 00:00:00", .type = "ONCE"});
  pqxx::result res = state.db_trans.select_all<Task>();

  std::vector<std::string> tasks;

  for (const auto &row : res) {
    int id = row["id"].as<int>();
    std::string name = row["name"].as<std::string>();
    bool done = row["done"].as<bool>();
    std::optional<std::string> deadline;
    if (!row["deadline"].is_null())
      deadline = row["deadline"].as<std::string>();
    std::string type = row["type"].as<std::string>();

    std::stringstream ss;

    ss << id << ' ' << name << ' ' << done << ' ';
    if (deadline)
      ss << *deadline << ' ';

    ss << type << '\n';

    tasks.push_back(ss.str());
  }

  return ftxui::Renderer([tasks] {
    ftxui::Elements elements;

    for (const auto &task : tasks) {
      elements.push_back(ftxui::text(task));
    }

    return ftxui::vbox(std::move(elements));
  });
}
