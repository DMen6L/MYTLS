add_library(todo_lib STATIC todo_app.cpp)
target_include_directories(todo_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(todo_lib PUBLIC ftxui::screen ftxui::dom ftxui::component)

# Standalone runner — lets you launch todo directly without the menu
add_executable(todo_app main.cpp)
target_link_libraries(todo_app PRIVATE todo_lib)
