#pragma once

#include <ftxui/component/component.hpp>

#include <string>

// Appliable input modes
enum class InputMode { Normal, Insert };

// State of the input mode and the data it holds
struct InputHandlerState {
  std::string data = "";
  InputMode mode = InputMode::Normal;
};

ftxui::Component MakeInputHandler(InputHandlerState &input_state, ftxui::Component inner);
