add_library(my_data
  src/my_data.cpp
)

target_link_libraries(my_data
  PRIVATE db
  main_functional
  app_state
  render_elements
  ftxui::screen
  ftxui::dom
  ftxui::component
)

target_include_directories(my_data PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
