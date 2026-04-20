#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

int main (int argc, char *argv[]) {
  auto screen = ftxui::ScreenInteractive::TerminalOutput();

  auto button = ftxui::Button("Exit", [&]{
      screen.Exit();
  });

  screen.Loop(button);

  return 0;
}
