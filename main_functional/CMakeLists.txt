add_library(main_functional
  events.cpp
)

target_include_directories(main_functional PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(main_functional PRIVATE
  db
  todo
  app_state
  ftxui::screen
  ftxui::dom
  ftxui::component
)
