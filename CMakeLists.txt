cmake_minimum_required(VERSION 3.20)

project(mytls LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SQLite3 REQUIRED)

add_executable(mytls
    src/main.c
    src/database.c
)

target_include_directories(mytls PRIVATE include)

target_link_libraries(mytls PRIVATE SQLite3::SQLite3)
