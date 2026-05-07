add_library(extras_lib STATIC input_handler.cpp)
target_include_directories(extras_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(extras_lib PUBLIC ftxui::screen ftxui::dom ftxui::component)
