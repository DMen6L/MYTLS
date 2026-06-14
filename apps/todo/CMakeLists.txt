add_library(todo
  src/todo.cpp
)

target_link_libraries(todo
  PRIVATE db
)

target_include_directories(todo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(todo PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component
)
