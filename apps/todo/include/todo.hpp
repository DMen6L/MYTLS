#ifndef TODO
#define TODO

#include "task.hpp"
#include "transactor.hpp"
#include <ftxui/component/app.hpp>
#include <string>
#include <vector>

struct TodoState {
  Transactor &db_trans;
  NewTask new_task;
  std::vector<std::string> tasks;
  std::string deadline_text;

  TodoState(Transactor &trans) : db_trans(trans) {}
};

ftxui::Component todo_render(TodoState &state);

#endif
