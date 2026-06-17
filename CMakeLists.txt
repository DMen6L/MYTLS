cmake_minimum_required(VERSION 3.20)
project(MyTLS VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(apps/todo)
add_subdirectory(apps/db)
add_subdirectory(external/FTXUI)
add_subdirectory(main_functional)
add_subdirectory(AppState)
add_subdirectory(render_elements)

add_executable(mytls main.cpp)

target_link_libraries(mytls
  PRIVATE 
    todo
    db
    main_functional
    app_state
    render_elements
    ftxui::screen
    ftxui::dom
    ftxui::component
)
