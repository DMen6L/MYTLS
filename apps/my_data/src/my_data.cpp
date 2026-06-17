#include "my_data.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <utility>

ftxui::Component MakeMyDataBody() {
  return ftxui::Renderer([&] {
    ftxui::Elements body;
    body.push_back(ftxui::text("Hello world"));

    return ftxui::window(ftxui::text("MY Data"), ftxui::vbox(std::move(body)));
  });
}
