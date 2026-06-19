add_library(render_elements
  todo/todo_render.cpp
  main/main_render.cpp
)

target_link_libraries(render_elements
  PRIVATE
  db
  app_state
  ftxui::screen
  ftxui::dom
  ftxui::component
)

target_include_directories(render_elements PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/todo
  ${CMAKE_CURRENT_SOURCE_DIR}/main
)
