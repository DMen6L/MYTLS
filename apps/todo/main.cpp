#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <iostream>

int main (int argc, char *argv[]) {
  using namespace ftxui;
  
  std::string field;
  std::string sbm;

  auto input = Input(&field, "Write something");

  // Output Screen
  auto screen = ScreenInteractive::Fullscreen();

  // Button
  auto btn = Button("Submit", [&] {
      sbm = "Typed: " + field;
      field = "";
  });

  auto exit_btn = Button("Exit", screen.ExitLoopClosure());

  //Base layout
  auto layout = Container::Vertical({ input, btn, exit_btn });
  
  // Renderer
  auto renderer = Renderer(layout, [&] {
      return vbox({
          text("Welcome!") | bold,
          separator(),
          input->Render(),
          btn->Render(),
          exit_btn->Render(),
          text(sbm),
      }) | border;
  });

  screen.Loop(renderer);
  return 0;
}
