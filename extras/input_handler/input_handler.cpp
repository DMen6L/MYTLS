#include "input_handler.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <string>
#include <string_view>

using namespace ftxui;

// To turn the input mode states into strings
std::string_view toString(const InputMode &mode) {
  switch (mode) {
  case InputMode::Normal:
    return "Normal";
  case InputMode::Insert:
    return "Insert";
  }

  return "ERROR";
}

Component MakeInputHandler(InputHandlerState &input_state, Component inner) {
  // renderer for the UI normal pane and the footer for input state
  auto renderer = Renderer(inner, [&input_state, inner] {
    return vbox({
        inner->Render() | flex,
        hbox({
            text(std::string(toString(input_state.mode))),
            text(input_state.data),
        }),
    });
  });

  // To catch input beforehand
  auto handler = CatchEvent(renderer, [&input_state](Event e) -> bool {
    if (input_state.mode == InputMode::Insert) {
      if (e == Event::Escape) {
        input_state.mode = InputMode::Normal;
        return true;
      }
      if (e == Event::Backspace) {
        if (!input_state.data.empty()) {
          input_state.data.pop_back();
        }
        return true;
      }
      if (e.is_character()) {
        input_state.data += e.character();
        return true;
      }
    }

    if (input_state.mode == InputMode::Normal) {
      if (e.is_character() && e.character() == "i") {
        input_state.mode = InputMode::Insert;
        return true;
      }
    }

    return false;
  });

  return handler;
}
