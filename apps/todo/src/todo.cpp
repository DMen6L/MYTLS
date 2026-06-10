#include "todo.hpp"
#include "db.hpp"
#include "task.hpp"
#include "transactor.hpp"

#include <cstdlib>
#include <ios>
#include <iostream>
#include <limits>
#include <pqxx/pqxx>
#include <string>

void todo_render(todo_state &state) {
  std::string conn_str = db_init_conn();
  Transactor db_trans(conn_str);

  db_trans.test_conn();
  db_trans.init_table<TaskTable>();

  int option = -1;

  while (true) {
    std::system("clear");

    std::cout << "Welcome to todo!\n";
    std::cout << "Choose action\n";
    std::cout << "0. add task\n";

    std::cin >> option;
    // ignore tailing endline
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (option == -1)
      break;

    std::system("clear");
    if (option == 0) {
      std::string new_task_name;
      std::cout << "Name of new task: ";
      std::getline(std::cin, new_task_name);

      std::system("clear");
      std::string deadline_input;
      std::cout << "deadline of the task(2026-12-31 00:00:00): ";
      std::getline(std::cin, deadline_input);

      db_trans.insert<NewTask>(
          NewTask{.name = new_task_name, .deadline = deadline_input});
    }
  }
}
