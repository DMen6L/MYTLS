cmake_minimum_required(VERSION 3.20)

project(mytls LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SQLite3 REQUIRED)
find_package(Curses REQUIRED)
find_path(FORM_INCLUDE_DIR NAMES form.h HINTS ${CURSES_INCLUDE_DIRS} REQUIRED)
find_library(FORM_LIBRARY NAMES form HINTS ${CURSES_LIBRARY_DIRS} REQUIRED)

add_executable(mytls
    src/main.c
    src/database.c
    src/tui.c
    src/task.c
)

target_include_directories(mytls PRIVATE include ${CURSES_INCLUDE_DIRS} ${FORM_INCLUDE_DIR})

target_link_libraries(mytls PRIVATE SQLite3::SQLite3 ${FORM_LIBRARY} ${CURSES_LIBRARIES})
