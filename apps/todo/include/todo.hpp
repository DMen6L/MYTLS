#ifndef TODO
#define TODO

#include <string>
#include <vector>

struct todo_state {
  std::vector<std::string> tasks;
};

void todo_render(todo_state &state);

#endif
