#include "db.hpp"
#include "todo.hpp"
#include "transactor.hpp"

#include "ftxui/component/app.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <vector>

int main() {
  using namespace ftxui;

  std::string conn_str = db_init_conn();
  Transactor trans(conn_str);
  TodoState todo_state(trans);

  auto screen = App::TerminalOutput();

  std::vector<std::string> entries = {"TODO"};
  int selected = 0;

  Component menu = Menu(&entries, &selected);
  Component todo = todo_render(todo_state);

  int page = 0;
  menu |= CatchEvent([&](Event event) {
    if (event == Event::Return) {
      page = 1;
      return true;
    }
    return false;
  });

  todo |= CatchEvent([&](Event event) {
    if (event == Event::Escape) {
      page = 0;
      return true;
    }
    return false;
  });

  auto container = Container::Tab({menu, todo}, &page);

  screen.Loop(container);

  return 0;
}
