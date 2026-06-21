#include "todo.hpp"
#include "AppState.hpp"
#include "daily_reports.hpp"
#include "daily_reports_entry.hpp"
#include "db.hpp"
#include "task.hpp"

#include <fcntl.h>
#include <format>
#include <ftxui/component/app.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <pqxx/pqxx>
#include <string>
#include <utility>
#include <vector>

ftxui::Component MakeTodoList(AppState &app_state) {
  return ftxui::Renderer([&] {
    ftxui::Elements task_list;
    int count = 0;

    for (const auto &task : app_state.tasks) {
      ftxui::Element status;

      if (task.total > 1 && !task.done) {
        status = ftxui::hbox(
            {ftxui::text(std::format("{}/{}", task.progress, task.total)),
             ftxui::gauge(task.get_progress()) |
                 ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 10) |
                 ftxui::color(ftxui::Color::Green)});
      } else {
        if (task.done)
          status = ftxui::text("󰱒 ") | ftxui::color(ftxui::Color::Green);
        else
          status = ftxui::text(" ");
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
        row |= ftxui::focus;
      }

      task_list.push_back(row);
      count++;
    }

    return ftxui::window(ftxui::text("TODO List"),
                         ftxui::vbox(std::move(task_list)) | ftxui::frame);
  });
}

ftxui::Component MakeTodoAdd(AppState &app_state) {
  auto name_field = ftxui::Input(&app_state.new_task.name, "Task Name");
  auto type_dropdown =
      ftxui::Dropdown(&app_state.type_entries, &app_state.selected_type);

  auto deadline_field =
      ftxui::Input(&app_state.temp_deadline, &app_state.deadline_placeholder);

  auto total_field =
      ftxui::Input(&app_state.temp_total, "number of total reps");
  auto progress_field =
      ftxui::Input(&app_state.temp_progress, "number of done reps");

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

ftxui::Component MakeTodoUpdate(AppState &app_state) {
  auto name_field = ftxui::Input(&app_state.edit_name, "Task name");
  auto type_dropdown =
      ftxui::Dropdown(&app_state.type_entries, &app_state.edit_type);

  auto deadline_field =
      ftxui::Input(&app_state.edit_deadline, &app_state.deadline_placeholder);

  auto total_field =
      ftxui::Input(&app_state.edit_total, "number of total reps");
  auto progress_field =
      ftxui::Input(&app_state.edit_progress, "number of done reps");

  auto submit = ftxui::Button("Submit", [&] {
    Task &task = app_state.tasks[app_state.editing_task_idx];
    UpdateTask upd_task = UpdateTask{
        .id = app_state.tasks[app_state.editing_task_idx].id,
    };
    if (task.name != app_state.edit_name)
      upd_task.name = app_state.edit_name;
    if (task_type_to_string(task.type) !=
        app_state.type_entries[app_state.edit_type])
      upd_task.type =
          task_type_from_string(app_state.type_entries[app_state.edit_type]);
    upd_task.deadline.set(
        task_type_from_string(app_state.type_entries[app_state.edit_type]),
        app_state.edit_deadline);
    if (task.total != std::stoi(app_state.edit_total))
      upd_task.total = std::stoi(app_state.edit_total);
    if (task.progress != std::stoi(app_state.edit_progress))
      upd_task.progress = std::stoi(app_state.edit_progress);

    app_state.trans.update<UpdateTask>(upd_task);
    app_state.ClearEditValues();
    app_state.NavigationBack();
  });

  return ftxui::Container::Vertical({name_field, type_dropdown, deadline_field,
                                     total_field, progress_field, submit});
}

ftxui::Component MakeDailyEntryList(AppState &app_state) {
  auto report_dates_dropdown = ftxui::Dropdown(&app_state.report_date_entries,
                                               &app_state.current_report);
  auto choose1 = ftxui::Button(
      "select", [&] { app_state.NavigationForward(Page::DailyReport); });

  return ftxui::Container::Vertical({
      ftxui::Container::Horizontal({
          report_dates_dropdown,
          choose1,
      }),
  });
}

ftxui::Component MakeDailyReport(AppState &app_state) {
  return ftxui::Renderer([&] {
    std::vector<DailyReportsEntry> entries =
        app_state.GetCurrentReportEntries();
    ftxui::Elements entry_list;
    int count = 0;

    for (const auto &entry : entries) {
      ftxui::Element status;

      if (entry.was_completed && entry.total > 1) {
        status = ftxui::text("󰸞 " + std::to_string(entry.total)) |
                 ftxui::color(ftxui::Color::Green);
      } else if (entry.was_completed) {
        status = ftxui::text("󰸞 ") | ftxui::color(ftxui::Color::Green);
      } else if (entry.progress == 0) {
        status = ftxui::text(" ") | ftxui::color(ftxui::Color::Red);
      } else {
        status = ftxui::text(std::format("{}/{}", entry.progress, entry.total));
      }
      auto row = ftxui::hbox({
          ftxui::text(std::format("{:>3}", count + 1)),
          ftxui::text(" | "),
          ftxui::filler(),
          ftxui::text(entry.task_name),
          ftxui::filler(),
          status,
      });

      if (count == app_state.current_entry) {
        row |= ftxui::color(ftxui::Color::Black);
        row |= ftxui::bgcolor(ftxui::Color::White);
        row |= ftxui::focus;
      }

      entry_list.push_back(row);
      count++;
    }

    return ftxui::window(
        ftxui::text(app_state.report_date_entries[app_state.current_report]),
        ftxui::vbox(std::move(entry_list)) | ftxui::xflex | ftxui::frame);
  });
}
