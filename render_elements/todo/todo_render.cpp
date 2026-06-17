#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
using namespace ftxui;

RadioboxOption TaskTypeOption() {
  RadioboxOption option;

  option.transform = [](const EntryState &state) { return hbox(); };

  return option;
}
