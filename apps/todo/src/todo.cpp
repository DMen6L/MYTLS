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

ftxui::Component MakeTaskInput(TodoState &state) {
  auto task_input = ftxui::Input(&state.new_task.name, "New Task");
  auto done_checkbox = ftxui::Checkbox("Done", &state.new_task.done);
  auto type_input = ftxui::Input(&state.new_task.type, "Type");

  auto deadline_input =
      ftxui::Input(&state.deadline_text, "YYYY-MM-DD HH:MM:SS");

  auto submit = ftxui::Button("Submit", [&] {
    if (state.deadline_text.empty())
      state.new_task.deadline = std::nullopt;
    else
      state.new_task.deadline = state.deadline_text;

    state.db_trans.insert<NewTask>(state.new_task);
    state.new_task = NewTask{};
    state.deadline_text.clear();
  });

  auto container = ftxui::Container::Vertical(
      {task_input, type_input, deadline_input, done_checkbox, submit});
  return container;
}

ftxui::Component todo_render(TodoState &state) {
  state.db_trans.init_table<TaskTable>();

  auto form = MakeTaskInput(state);

  auto list = ftxui::Renderer([&] {
    pqxx::result res = state.db_trans.select_all<TaskTable>();

    ftxui::Elements elements;

    for (const auto &row : res) {
      std::stringstream ss;

      ss << row["id"].as<int>() << ' ' << row["name"].as<std::string>() << ' '
         << row["done"].as<bool>() << ' ';

      if (!row["deadline"].is_null())
        ss << row["deadline"].as<std::string>() << ' ';

      ss << row["type"].as<std::string>();

      elements.push_back(ftxui::text(ss.str()));
    }

    return ftxui::vbox(std::move(elements));
  });

  return ftxui::Container::Vertical({list, form});
}
