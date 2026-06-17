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
#include <iostream>
#include <optional>
#include <pqxx/pqxx>
#include <sstream>

ftxui::Component MakeTodoList(AppState &app_state) {
  return ftxui::Renderer([&] {
    ftxui::Elements task_list;
    int count = 0;

    for (const Task &task : app_state.tasks) {
      auto row = ftxui::hbox({ftxui::text(task.done ? "[x]" : "[ ]"),
                              ftxui::text(task.name), ftxui::filler(),
                              ftxui::text(task_type_to_string(task.type))});

      if (app_state.current_todo == count) {
        row |= ftxui::inverted;
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
  auto deadline_field =
      ftxui::Input(&app_state.temp_deadline, "YYYY-MM-DD HH:MM:SS");
  auto done_checkbox = ftxui::Checkbox("Done", &app_state.new_task.done);

  auto submit = ftxui::Button("Submit", [&] {
    app_state.ReadTempValues();
    pqxx::result res = app_state.trans.insert<NewTask>(app_state.new_task);
    app_state.tasks.push_back(task_from_row(res[0]));
    app_state.ClearTempValues();
    app_state.NavigationBack();
  });

  return ftxui::Container::Vertical(
      {name_field, type_dropdown, deadline_field, done_checkbox, submit});
}
