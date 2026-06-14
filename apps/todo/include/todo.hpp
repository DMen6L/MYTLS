#ifndef TODO
#define TODO

#include "transactor.hpp"
#include <ftxui/component/app.hpp>
#include <string>
#include <vector>

struct TodoState {
  Transactor &db_trans;
  std::vector<std::string> tasks;

  TodoState(Transactor &trans) : db_trans(trans) {}
};

ftxui::Component todo_render(Transactor &trans);

#endif
