#include "todo_app.hpp"

#include <ftxui/component/screen_interactive.hpp>
int main(int argc, char *argv[]) {
  ToDoState state;

  auto app = MakeToDoApp(state);

  auto screen = ftxui::ScreenInteractive::TerminalOutput();
  screen.Loop(app);

  return 0;
}
