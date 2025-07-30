#include "utils.hpp"


#include <ranges>

using namespace ftxui;

std::vector<std::string> split_csv_line(std::string_view line, char delimiter /*= ','*/)
{
    return line
        | std::views::split(delimiter)
        | std::views::transform([](auto&& subrange) {
              return std::string(&*subrange.begin(), std::ranges::distance(subrange));
          })
        | std::ranges::to<std::vector<std::string>>();
}

std::vector<std::string_view> split_csv_line_view(std::string_view line, char delimiter /*= ','*/)
{
    std::vector<std::string_view> result;
    for (auto&& subrange : std::views::split(line, delimiter))
    {
        result.emplace_back(&*subrange.begin(), std::ranges::distance(subrange));
    }

    return result;
}

std::string EventToString(const Event& event) {
    if (event == Event::Return) return "return";
    if (event == Event::Escape) return "escape";
    if (event == Event::Tab) return "tab";
    if (event == Event::Backspace) return "backspace";
    if (event == Event::Delete) return "delete";
    if (event == Event::ArrowUp) return "arrowup";
    if (event == Event::ArrowDown) return "arrowdown";
    if (event == Event::ArrowLeft) return "arrowleft";
    if (event == Event::ArrowRight) return "arrowright";
    if (event == Event::Home) return "home";
    if (event == Event::End) return "end";
    if (event == Event::PageUp) return "pageup";
    if (event == Event::PageDown) return "pagedown";

    // Handle single characters
    if (event.is_character()) {
        return event.character();
    }

    if (event == Event::F1) return "f1";
    if (event == Event::F2) return "f2";
    if (event == Event::F3) return "f3";
    if (event == Event::F4) return "f4";
    if (event == Event::F5) return "f5";
    if (event == Event::F6) return "f6";
    if (event == Event::F7) return "f7";
    if (event == Event::F8) return "f8";
    if (event == Event::F9) return "f9";
    if (event == Event::F10) return "f10";
    if (event == Event::F11) return "f11";
    if (event == Event::F12) return "f12";

    // Return a default string for unknown events
    return "unknown";
}
