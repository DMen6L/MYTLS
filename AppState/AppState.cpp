#include "AppState.hpp"

std::string page_to_string(const Page &page) {
  switch (page) {
  case Page::MainMenu:
    return "MainMenu";
  case Page::TodoList:
    return "TodoList";
  case Page::TodoAdd:
    return "TodoAdd";
  case Page::TodoUpdate:
    return "TodoUpdate";
  case Page::MyData:
    return "MyData";
  case Page::Reports:
    return "Reports";
  case Page::DailyReport:
    return "DailyReport";
  }

  return "MainMenu";
}

Page string_to_page(const std::string &page_str) {
  if (page_str == "MainMenu")
    return Page::MainMenu;
  if (page_str == "TodoList")
    return Page::TodoList;
  if (page_str == "TodoAdd")
    return Page::TodoAdd;
  if (page_str == "TodoUpdate")
    return Page::TodoUpdate;
  if (page_str == "MyData")
    return Page::MyData;
  if (page_str == "Reports")
    return Page::Reports;
  if (page_str == "DailyReport")
    return Page::DailyReport;

  return Page::MainMenu;
}
