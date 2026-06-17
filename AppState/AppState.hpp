#ifndef APPSTATE
#define APPSTATE

#include "task.hpp"
#include "transactor.hpp"
#include <ftxui/component/app.hpp>
#include <stack>
#include <string>
#include <vector>

enum class Page { MainMenu = 0, TodoList = 1, TodoAdd = 2, MyData = 3 };

std::string page_to_string(const Page &page);
Page string_to_page(const std::string &page_str);

struct AppState {
  // GLobal
  Transactor &trans;
  std::vector<std::string> main_menu_entries = {"TodoList", "MyData"};
  int man_menu_selected = 0;
  int current_page = 0;
  std::stack<Page> navigation_stack;

  // Todo list stuff
  std::vector<Task> tasks; // Saving tasks for transactions with them
  // Todo list navigation
  int current_todo = 0;

  NewTask new_task;
  std::string temp_deadline;
  std::vector<std::string> type_entries;
  int selected_type = 0;

  explicit AppState(Transactor &db_trans) : trans(db_trans) {
    navigation_stack.push(Page::MainMenu);
  }

  // Routine for initiation of all stuff
  void initiation_routine() {
    this->trans.init_table<TaskTable>();
    pqxx::result res = this->trans.select_all<TaskTable>();

    for (const auto &row : res) {
      this->tasks.push_back(task_from_row(row));
    }

    for (const TaskType &type : AllTaskTypes) {
      type_entries.push_back(task_type_to_string(type));
    }
  }

  void UpdateCurrentPage() {
    this->current_page = static_cast<int>(this->navigation_stack.top());
  }

  void NavigationForward(Page &chosen_page) {
    this->navigation_stack.push(chosen_page);
    this->UpdateCurrentPage();
  }

  void NavigationBack() {
    this->navigation_stack.pop();
    this->UpdateCurrentPage();
  }

  Task GetCurrentTask() { return this->tasks[this->current_todo]; }

  void ReadTempValues() {
    if (this->temp_deadline.empty())
      this->new_task.deadline = std::nullopt;
    else
      this->new_task.deadline = this->temp_deadline;

    this->new_task.type = AllTaskTypes[this->selected_type];
  }

  void ClearTempValues() {
    this->new_task = NewTask{};
    this->temp_deadline.clear();
    this->selected_type = 0;
  }
};

#endif // !APPSTATE
