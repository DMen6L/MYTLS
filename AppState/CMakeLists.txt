add_library(app_state INTERFACE)

target_include_directories(app_state INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(app_state INTERFACE 
  db
  ftxui::screen
  ftxui::dom
  ftxui::component
)
