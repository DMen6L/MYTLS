#include "todo.hpp"
#include "db.hpp"
#include "task.hpp"

#include <fcntl.h>
#include <ftxui/component/app.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <optional>
#include <pqxx/pqxx>
#include <sstream>

ftxui::Component MakeTodoList(AppState &app_state) {
  return ftxui::Renderer([&] {
    ftxui::Elements task_list;
    task_list.push_back(ftxui::text("TODO List"));
    task_list.push_back(ftxui::separator());
    int count = 0;

    for (const Task &task : app_state.tasks) {
      std::stringstream ss;

      ss << count + 1 << ' ' << task.name << ' ' << task.type << ' ';

      if (task.deadline)
        ss << *task.deadline << ' ';

      ss << task.done;

      auto task_text = ftxui::text(ss.str());

      if (app_state.current_todo == count) {
        task_text |= ftxui::color(ftxui::Color::Black);
        task_text |= ftxui::bgcolor(ftxui::Color::White);
      }

      task_list.push_back(task_text);
      count++;
    }

    return ftxui::vbox(std::move(task_list));
  });
}

ftxui::Component MakeTodoAdd(AppState &app_state) {

  auto name_field = ftxui::Input(&app_state.new_task.name, "Task Name");
  auto type_field = ftxui::Input(&app_state.new_task.type, "Task Type");
  auto deadline_field =
      ftxui::Input(&app_state.temp_deadline, "YYYY-MM-DD HH:MM:SS");
  auto done_checkbox = ftxui::Checkbox("Done", &app_state.new_task.done);

  if (app_state.temp_deadline.empty())
    app_state.new_task.deadline = std::nullopt;
  else
    app_state.new_task.deadline = app_state.temp_deadline;

  auto submit = ftxui::Button("Submit", [&] {
    pqxx::result res = app_state.trans.insert<NewTask>(app_state.new_task);
    app_state.tasks.push_back(task_from_row(res[0]));
    app_state.new_task = NewTask{};
    app_state.NavigationBack();
  });

  return ftxui::Container::Vertical(
      {name_field, type_field, deadline_field, done_checkbox, submit});
}
