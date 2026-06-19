#include "AppState.hpp"
#include "db.hpp"
#include "events.hpp"
#include "main_render.hpp"
#include "my_data.hpp"
#include "task.hpp"
#include "todo.hpp"
#include "transactor.hpp"

#include <ftxui/component/app.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

int main() {
  using namespace ftxui;
  std::string conn_str = db_init_conn(); // Connection string to database
  Transactor trans(conn_str);            // Transactor for database actions

  AppState app_state(trans); // State of the application
  app_state.initiation_routine();

  auto main_menu = Menu(&app_state.main_menu_entries,
                        &app_state.main_menu_selected, MainMenuStyle());
  auto todo_list = MakeTodoList(app_state);
  auto todo_add = MakeTodoAdd(app_state);
  auto my_data = MakeMyDataBody();

  auto container = Container::Tab({main_menu, todo_list, todo_add, my_data},
                                  &app_state.current_page);
  container |= CatchEvent(MakeInputHandler(app_state));

  auto screen = App::TerminalOutput();

  screen.Loop(container);

  return 0;
}
