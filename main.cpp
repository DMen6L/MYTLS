#include "todo.hpp"
#include <iostream>

int main() {
  int choice = -1;
  todo_state todo;

  while (true) {
    std::system("clear");

    std::cout << "Please choose what operation to do:\n";
    std::cout << "0. todo list\n";
    std::cout << "current choice: ";

    std::cin >> choice;

    if (choice == -1) {
      std::cout << "Ending program!\n";
      return 1;
    }
    if (choice == 0)
      todo_render(todo);
  }

  return 0;
}
