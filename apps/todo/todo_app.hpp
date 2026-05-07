#pragma once

#include <ftxui/component/component.hpp>

#include <string>
#include <vector>

// State of the todo app
struct ToDoState {
  std::vector<std::string> tasks;
  std::string input;
};

// Builder of the todo app
ftxui::Component MakeToDoApp(ToDoState &state);
