#include <ftxui/component/event.hpp>
#include <functional>

#include "AppState.hpp"
#include "events.hpp"

// TODO: main menu selections
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
        app_state.navigation_stack.push(Page::TodoList);
        app_state.UpdateCurrentPage();
        return true;
      }
      break;
    case Page::TodoList:
      if (event == ftxui::Event::Character('j') ||
          event == ftxui::Event::ArrowDown) {
        if (app_state.current_todo >= app_state.total_todos - 1)
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
        app_state.navigation_stack.push(Page::TodoAdd);
        app_state.UpdateCurrentPage();
        return true;
      }
      if (event == ftxui::Event::CtrlD) {
        app_state.trans.delete_by_id<Task>(app_state.GetCurrentTask().id);
        std::erase_if(app_state.tasks, [&](const Task &task) {
          return task.id == app_state.GetCurrentTask().id;
        });
      }
      break;

    case Page::TodoAdd:
      break;
    }

    return false;
  };
}
