#ifndef TODO
#define TODO

#include "AppState.hpp"

#include <ftxui/component/app.hpp>

ftxui::Component MakeTodoList(AppState &app_state);
ftxui::Component MakeTodoAdd(AppState &app_state);
ftxui::Component MakeTodoUpdate(AppState &app_state);
ftxui::Component MakeDailyEntryList(AppState &app_state);
ftxui::Component MakeDailyReport(AppState &app_state);

#endif
