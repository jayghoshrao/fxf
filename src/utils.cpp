#include "utils.hpp"

#include <ranges>

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
