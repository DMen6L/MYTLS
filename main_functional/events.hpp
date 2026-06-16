#ifndef MAINFUNC
#define MAINFUNC

#include "AppState.hpp"

#include <ftxui/component/event.hpp>
#include <functional>

std::function<bool(ftxui::Event)> MakeInputHandler(AppState &app_state);

#endif
