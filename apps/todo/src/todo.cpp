#include "todo.hpp"
#include "AppState.hpp"
#include "db.hpp"
#include "task.hpp"
#include "todo_render.hpp"

#include <fcntl.h>
#include <ftxui/component/app.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <pqxx/pqxx>

ftxui::Component MakeTodoList(AppState &app_state) {
  return ftxui::Renderer([&] {
    ftxui::Elements task_list;
    int count = 0;

    for (const auto &task : app_state.tasks) {
      ftxui::Element status;

      if (task.total > 1) {
        status = ftxui::gauge(task.get_progress()) |
                 ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 10) |
                 ftxui::color(ftxui::Color::Green);
      } else {
        status = ftxui::text(task.done ? "[x]" : "[ ]");
      }

      auto row = ftxui::hbox({
          status,
          ftxui::text(task.name),
          ftxui::filler(),
          // ------ Extra task info -------
          ftxui::hbox({
              ftxui::text(task.deadline.to_string()) | ftxui::italic |
                  ftxui::color(ftxui::Color::GrayDark),
              ftxui::text("  "),
              ftxui::text(task_type_to_string(task.type)),
          }),
      });

      if (app_state.current_todo == count) {
        row |= ftxui::color(ftxui::Color::Black);
        row |= ftxui::bgcolor(ftxui::Color::White);
      }

      task_list.push_back(row);
      count++;
    }

    return ftxui::window(ftxui::text("TODO List"),
                         ftxui::vbox(std::move(task_list)));
  });
}

ftxui::Component MakeTodoAdd(AppState &app_state) {
  auto name_field = ftxui::Input(&app_state.new_task.name, "Task Name");
  auto type_dropdown =
      ftxui::Dropdown(&app_state.type_entries, &app_state.selected_type);
  switch (
      task_type_from_string(app_state.type_entries[app_state.selected_type])) {
  case TaskType::ONCE:
    app_state.deadline_placeholder = "YYYY-MM-DD";
    break;
  case TaskType::DAILY:
    app_state.deadline_placeholder = "HH:MM";
    break;
  case TaskType::WEEKLY:
    app_state.deadline_placeholder = "1-7";
    break;
  case TaskType::MONTHLY:
    app_state.deadline_placeholder = "1-31";
    break;
  case TaskType::LONGTERM:
    app_state.deadline_placeholder = "YYYY-MM-DD";
    break;
  default:
    app_state.deadline_placeholder = "";
  }

  auto deadline_field =
      ftxui::Input(&app_state.temp_deadline, &app_state.deadline_placeholder);

  auto total_field = Input(&app_state.temp_total, "number of total reps");
  auto progress_field = Input(&app_state.temp_progress, "number of done reps");

  auto submit = ftxui::Button("Submit", [&] {
    app_state.ReadTempValues();
    pqxx::result res = app_state.trans.insert<NewTask>(app_state.new_task);
    app_state.tasks.push_back(task_from_row(res[0]));
    app_state.ClearTempValues();
    app_state.NavigationBack();
  });

  return ftxui::Container::Vertical({name_field, type_dropdown, deadline_field,
                                     total_field, progress_field, submit});
}
