add_executable(todo_app main.cpp)
target_link_libraries(todo_app PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component
)
