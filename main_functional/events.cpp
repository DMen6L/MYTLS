#include <ftxui/component/event.hpp>
#include <functional>

#include "AppState.hpp"
#include "events.hpp"
#include "task.hpp"
#include "todo.hpp"

std::function<bool(ftxui::Event)> MakeInputHandler(AppState &app_state) {
  return [&](ftxui::Event event) {
    // if escape go back in the pages hierarchy
    if (event == ftxui::Event::Escape && app_state.current_page > 0) {
      app_state.NavigationBack();
      return true;
    }

    switch (app_state.navigation_stack.top()) {
    case Page::MainMenu:
      if (event == ftxui::Event::Return) {
        // Add new top page and update current page
        Page chosen_page = string_to_page(
            app_state.main_menu_entries[app_state.main_menu_selected]);
        app_state.NavigationForward(chosen_page);
        return true;
      }
      break;
    case Page::TodoList:
      if (event == ftxui::Event::Character('j') ||
          event == ftxui::Event::ArrowDown) {
        if (app_state.current_todo >= app_state.tasks.size() - 1)
          return false;
        app_state.current_todo++;
        return true;
      }
      if (event == ftxui::Event::Character('k') ||
          event == ftxui::Event::ArrowUp) {
        if (app_state.current_todo <= 0)
          return false;
        app_state.current_todo--;
        return true;
      }

      if (event == ftxui::Event::CtrlA) {
        app_state.NavigationForward(Page::TodoAdd);
        return true;
      }
      if (event == ftxui::Event::CtrlD) {
        app_state.trans.delete_by_id<Task>(app_state.GetCurrentTask().id);
        std::erase_if(app_state.tasks, [&](const Task &task) {
          return task.id == app_state.GetCurrentTask().id;
        });
        return true;
      }
      if (event == ftxui::Event::CtrlF) {
        Task &task = app_state.tasks[app_state.current_todo];
        if (task.progress >= task.total) {
          task.progress = 0;

          task.done = false;
        } else {
          task.progress++;
          if (task.progress == task.total)
            task.done = true;
        }
        app_state.trans.update<UpdateTask>(UpdateTask{
            .id = task.id,
            .done = task.done,
            .deadline = task.deadline,
            .progress = task.progress,
        });

        return true;
      }
      if (event == ftxui::Event::CtrlAltF) {
        Task &task = app_state.tasks[app_state.current_todo];
        if (task.progress == task.total) {
          task.progress--;
          task.done = false;
        }
        if (task.progress > 0)
          task.progress--;

        app_state.trans.update<UpdateTask>(UpdateTask{
            .id = task.id,
            .done = task.done,
            .deadline = task.deadline,
            .progress = task.progress,
        });

        return true;
      }
      if (event == ftxui::Event::Return) {
        app_state.UpdateEditingTask();
        app_state.NavigationForward(Page::TodoUpdate);

        return true;
      }
      break;

    case Page::TodoAdd:
      break;

    case Page::TodoUpdate:
      break;

    case Page::MyData:
      break;
    }

    return false;
  };
}
