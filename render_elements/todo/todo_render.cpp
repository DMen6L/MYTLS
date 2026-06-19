#include "AppState.hpp"
#include "task.hpp"

#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
using namespace ftxui;

InputOption AddDeadlineInputStyle(AppState &app_state) {
  InputOption option;

  switch (
      task_type_from_string(app_state.type_entries[app_state.selected_type])) {
  case TaskType::ONCE:
    option.placeholder = "%YYYY-%MM-%DD";
    break;
  case TaskType::DAILY:
    option.placeholder = "%HH:%MM";
    break;
  case TaskType::WEEKLY:
    option.placeholder = "1=Monday...";
    break;
  case TaskType::MONTHLY:
    option.placeholder = "1,2,3...";
    break;
  case TaskType::LONGTERM:
    option.placeholder = "";
    break;
  }

  return option;
}
