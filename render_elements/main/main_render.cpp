#include "main_render.hpp"
#include "ftxui/dom/elements.hpp"

#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

MenuOption MainMenuStyle() {
  MenuOption option;

  option.entries_option.transform = [](const EntryState &state) {
    auto menu_entry =
        hbox({text(state.active ? " " : " "), text(state.label)});

    return state.active ? menu_entry | inverted | bold : menu_entry;
  };

  return option;
}
