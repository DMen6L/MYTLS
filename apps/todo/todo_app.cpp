#include "todo_app.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

// Todo app builder definitions
// TODO: Redefine the builder for todo app, enhance
Component MakeToDoApp(ToDoState &state) {
  // Input for the new task
  auto input = Input(&state.input, "New Task");

  auto add_button = Button("Add", [&] {
    if (!state.input.empty()) {
      state.tasks.push_back(state.input);
      state.input.clear();
    }
  });

  auto container = Container::Vertical({
      input,
      add_button,
  });

  auto renderer = Renderer(container, [input, add_button, &state] {
    Elements task_list;

    for (const auto &task : state.tasks) {
      task_list.push_back(text("- " + task));
    }

    return vbox({
               text("To Do App") | bold | center,
               separator(),
               hbox({
                   text("Task: "),
                   input->Render() | flex,
                   add_button->Render(),
               }),
               separator(),
               vbox(task_list),
           }) |
           border;
  });

  return renderer;
}
