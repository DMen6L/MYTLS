add_library(db
  src/database/db.cpp
  src/extra/env.cpp
  src/extra/postgrsql.cpp
  src/repository_headers/task.cpp
)

find_package(libpqxx REQUIRED)
target_link_libraries(db
  PRIVATE libpqxx::pqxx
)

target_include_directories(db PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/database
  ${CMAKE_CURRENT_SOURCE_DIR}/src/extra
  ${CMAKE_CURRENT_SOURCE_DIR}/src/repository_headers
)
