#ifndef APPSTATE
#define APPSTATE

#include "task.hpp"
#include "transactor.hpp"

#include <ftxui/component/app.hpp>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

enum class Page {
  MainMenu = 0,
  TodoList = 1,
  TodoAdd = 2,
  TodoUpdate = 3,
  MyData = 4,
};

std::string page_to_string(const Page &page);
Page string_to_page(const std::string &page_str);

struct AppState {
  // Global
  Transactor &trans;
  std::vector<std::string> main_menu_entries = {"TodoList", "MyData"};
  int main_menu_selected = 0;
  int current_page = 0;
  std::stack<Page> navigation_stack;

  // Todo list stuff
  std::vector<Task> tasks; // Saving tasks for transactions with them
  // Todo list navigation
  int current_todo = 0;

  NewTask new_task;
  std::string deadline_placeholder = "YYYY-MM-DD";
  std::string temp_deadline;
  std::vector<std::string> type_entries;
  int selected_type = 0;
  std::string temp_total = "1";
  std::string temp_progress = "0";

  int editing_task_idx;
  std::string edit_name;
  int edit_type;
  std::string edit_deadline;
  std::string edit_total;
  std::string edit_progress;

  explicit AppState(Transactor &db_trans) : trans(db_trans) {
    navigation_stack.push(Page::MainMenu);
  }

  void SortTasks() {
    std::sort(this->tasks.begin(), this->tasks.end(),
              [](const Task &a, const Task &b) {
                if (a.type != b.type)
                  return task_type_rank(a.type) < task_type_rank(b.type);

                bool a_set = a.deadline.is_set();
                bool b_set = b.deadline.is_set();
                if (a_set != b_set)
                  return a_set;

                if (a.deadline < b.deadline)
                  return true;

                if (a.done != b.done)
                  return a.done;

                return a.id < b.id;
              });
  }

  // Routine for initiation of all stuff
  void initiation_routine() {
    this->trans.init_table<TaskTable>();
    pqxx::result res = this->trans.select_all<TaskTable>();

    for (const auto &row : res) {
      this->tasks.push_back(task_from_row(row));
    }

    this->SortTasks();

    for (const TaskType &type : AllTaskTypes) {
      type_entries.push_back(task_type_to_string(type));
    }
  }

  void UpdateCurrentPage() {
    if (this->navigation_stack.top() == Page::TodoList) {
      this->SortTasks();
    }
    this->current_page = static_cast<int>(this->navigation_stack.top());
  }

  void UpdateEditingTask() {
    Task &edit_task = this->tasks[this->current_todo];
    this->editing_task_idx = this->current_todo;
    this->edit_name = edit_task.name;
    this->edit_type = 0;
    this->edit_deadline = edit_task.deadline.to_string();
    this->edit_total = std::to_string(edit_task.total);
    this->edit_progress = std::to_string(edit_task.progress);
  }

  void NavigationForward(const Page &chosen_page) {
    this->navigation_stack.push(chosen_page);
    this->UpdateCurrentPage();
  }

  void NavigationBack() {
    this->navigation_stack.pop();
    this->UpdateCurrentPage();
  }

  Task GetCurrentTask() const { return this->tasks[this->current_todo]; }

  void ReadTempValues() {
    this->new_task.type = AllTaskTypes[this->selected_type];
    this->new_task.deadline.set(this->new_task.type, this->temp_deadline);

    // try catch for issue cases
    int total = 1;
    try {
      total = std::stoi(this->temp_total);
      if (total < 1)
        total = 1;
    } catch (const std::invalid_argument &) {
      total = 1;
    } catch (const std::out_of_range &) {
      total = 1;
    }
    this->new_task.total = total;

    int progress = 0;
    try {
      progress = std::stoi(this->temp_progress);
      if (progress < 1)
        progress = 1;
    } catch (const std::invalid_argument &) {
      progress = 1;
    } catch (const std::out_of_range &) {
      progress = 1;
    }
    this->new_task.progress = progress;
  }

  void ClearEditValues() {
    Task &edit_task = this->tasks[this->editing_task_idx];
    edit_task.name = this->edit_name;
    edit_task.type = task_type_from_string(this->type_entries[this->edit_type]);
    edit_task.deadline.set(
        task_type_from_string(this->type_entries[this->edit_type]),
        this->edit_deadline);
    edit_task.total = std::stoi(this->edit_total);
    edit_task.progress = std::stoi(this->edit_progress);
  }

  void ClearTempValues() {
    this->new_task = NewTask{};
    this->temp_deadline.clear();
    this->selected_type = 0;
  }
};

#endif // !APPSTATE
