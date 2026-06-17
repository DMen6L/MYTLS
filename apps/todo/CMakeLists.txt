add_library(todo
  src/todo.cpp
)

target_link_libraries(todo
  PRIVATE db
  main_functional
  app_state
  render_elements
  ftxui::screen
  ftxui::dom
  ftxui::component
)

target_include_directories(todo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
