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

  return Page::MainMenu;
}
