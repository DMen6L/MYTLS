add_library(app_state 
  AppState.cpp
)

target_include_directories(app_state PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(app_state PRIVATE
  db
  ftxui::screen
  ftxui::dom
  ftxui::component
)
