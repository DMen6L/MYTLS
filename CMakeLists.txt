cmake_minimum_required(VERSION 3.20)
project(MyTLS VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(apps/todo)
add_subdirectory(apps/db)

add_executable(mytls main.cpp)

target_link_libraries(mytls
  PRIVATE 
    todo
    db
)
