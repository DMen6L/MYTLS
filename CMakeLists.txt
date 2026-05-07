cmake_minimum_required(VERSION 3.16)
project(MyTLS VERSION 0.1)

add_subdirectory(external/ftxui)
add_subdirectory(apps/todo)
add_subdirectory(extras/input_handler)

add_executable(mytls main.cpp)

target_link_libraries(mytls PRIVATE
  todo_lib
  extras_lib
  ftxui::screen ftxui::dom ftxui::component
)
