#pragma once

#include <vector>
#include <ranges>

struct Table
{
    std::vector<std::vector<std::string>> data;

    Table(size_t N) : data(N) {}

    auto& operator[](size_t idx) { return data[idx];}

    auto add_line(std::string_view line, char delimiter = ',')
    {
        for(const auto& [col, token]: std::views::zip(data, line | std::views::split(delimiter)))
        {
            col.emplace_back(&*token.begin(), std::ranges::distance(token));
        }
    }

    std::vector<std::string> get_row(size_t idx)
    {
        std::vector<std::string> result;
        for(const auto& col : data)
        {
            result.emplace_back(col[idx]);
        }
        return result;
    }

};

namespace io{
    Table read_table(std::string_view filename, char delimiter=',');
}
