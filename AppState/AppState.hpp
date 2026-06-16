#ifndef APPSTATE
#define APPSTATE

#include "task.hpp"
#include "transactor.hpp"
#include <ftxui/component/app.hpp>
#include <stack>

enum class Page { MainMenu = 0, TodoList = 1, TodoAdd = 2 };

struct AppState {
  // GLobal
  Transactor &trans;
  int current_page = 0;
  std::stack<Page> navigation_stack;

  // Todo list stuff
  std::vector<Task> tasks; // Saving tasks for transactions with them
  // Todo list navigation
  int current_todo = 0;
  size_t total_todos = 0;

  NewTask new_task;
  std::string temp_deadline;

  explicit AppState(Transactor &db_trans) : trans(db_trans) {
    navigation_stack.push(Page::MainMenu);
  }

  // Routine for initiation of all stuff
  void initiation_routine() {
    this->trans.init_table<TaskTable>();
    pqxx::result res = this->trans.select_all<TaskTable>();
    this->total_todos = res.size();

    for (const auto &row : res) {
      this->tasks.push_back(task_from_row(row));
    }
  }

  void UpdateCurrentPage() {
    this->current_page = static_cast<int>(this->navigation_stack.top());
  }

  void NavigationBack() {
    this->navigation_stack.pop();
    this->UpdateCurrentPage();
  }

  Task GetCurrentTask() { return this->tasks[this->current_todo]; }
};

#endif // !APPSTATE
