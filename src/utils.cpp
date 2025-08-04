#include "utils.hpp"

#include <filesystem>
#include <ranges>
#include <memory>
#include <numeric>

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

std::string ExecAndCapture(const std::string& cmd) {
    char buffer[128];
    std::string result = "";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer, sizeof buffer, pipe.get()) != nullptr) {
        result += buffer;
    }

    return result;
}

std::string substitute_template(const std::string& template_str, const std::vector<std::string>& data) {
    std::string result = template_str;

    // Create joined string for {} placeholder
    std::string joined_data;
    if (!data.empty()) {
        joined_data = std::accumulate(data.begin() + 1, data.end(), data[0], 
            [](const std::string& acc, const std::string& s) {
                return acc + " | " + s;
            });
    }

    // Replace {} with joined data
    size_t pos = 0;
    while ((pos = result.find("{}", pos)) != std::string::npos) {
        result.replace(pos, 2, joined_data);
        pos += joined_data.length();
    }

    // Replace numbered placeholders {0}, {1}, {2}, etc.
    for (size_t i = 0; i < data.size(); ++i) {
        std::string placeholder = "{" + std::to_string(i) + "}";
        pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), data[i]);
            pos += data[i].length();
        }
    }

    return result;
}


std::string trim(const std::string& str) {
    const auto first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos)
        return ""; // all whitespace
    const auto last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}
