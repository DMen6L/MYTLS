#pragma once

#include <ftxui/component/component.hpp>

#include <string>
#include <vector>

struct ToDoState {
  std::vector<std::string> tasks;
  std::string input;
};

ftxui::Component MakeToDoApp(ToDoState& state);
