#include "apps/todo/todo_app.hpp"
#include "extras/input_handler/input_handler.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

enum class View { Menu, Todo, Quit };

int main(int argc, char *argv[]) {
  ToDoState todo_state;
  auto todo_app = MakeToDoApp(todo_state);

  std::vector<std::string> entries = {"Todo", "Exit"};
  int selected = 0;
  View current = View::Menu;

  auto screen = ftxui::ScreenInteractive::Fullscreen();

  auto menu = ftxui::Menu(&entries, &selected);

  auto launcher = ftxui::CatchEvent(menu, [&](ftxui::Event e) {
    if (e == ftxui::Event::Return) {
      if (selected == 0)
        current = View::Todo;
      if (selected == 1)
        current = View::Quit;
      screen.ExitLoopClosure()();
      return true;
    }
    return false;
  });

  InputHandlerState input_state;
  auto handler = MakeInputHandler(input_state, launcher);

  auto menu_renderer = ftxui::Renderer(handler, [&] {
    return ftxui::vbox({
               ftxui::text("MyTLS - Tools") | ftxui::bold | ftxui::center,
               ftxui::separator(),
               handler->Render() | ftxui::flex,
           }) |
           ftxui::border;
  });

  // Each iteration runs one screen; ExitLoopClosure sets current and breaks out
  while (current != View::Quit) {
    if (current == View::Menu) {
      screen.Loop(menu_renderer);
    } else if (current == View::Todo) {
      // Wrap todo_app so Escape returns to menu
      auto wrapped = ftxui::CatchEvent(todo_app, [&](ftxui::Event e) {
        if (e == ftxui::Event::Escape) {
          current = View::Menu;
          screen.ExitLoopClosure()();
          return true;
        }
        return false;
      });
      screen.Loop(wrapped);
    }
  }

  return 0;
}
