#include "db.hpp"
#include "todo.hpp"
#include "transactor.hpp"

#include "ftxui/component/app.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <vector>

int main() {
  using namespace ftxui;

  // Basic configurations
  std::string conn_str = db_init_conn();
  Transactor trans(conn_str);
  TodoState todo_state(trans);

  auto screen = App::TerminalOutput();

  std::vector<std::string> entries = {"TODO"};
  int selected = 0;

  auto menu = Menu(&entries, &selected);
  auto task_list = MakeTaskList(todo_state);
  auto task_form = MakeTaskInput(todo_state);

  int page = 0;

  auto container = Container::Tab({menu, task_list, task_form}, &page);
  container |= CatchEvent([&](Event event) {
    if (page == 0 && event == Event::Return) {
      page = 1;
      return true;
    }
    if (page == 1 && event == Event::Character('a')) {
      page = 2;
      return true;
    }
    if (page > 0 && event == Event::Escape) {
      page--;
      return true;
    }

    return false;
  });

  screen.Loop(container);

  return 0;
}
