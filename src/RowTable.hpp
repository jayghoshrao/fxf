#pragma once

#include <vector>
#include <ranges>
#include <fstream>

#include "utils.hpp"

struct RowTable
{
    using row_t = std::vector<std::string>;

    std::vector<row_t> data;

    auto& operator[](size_t idx) { return data[idx]; }
    const auto& operator[](size_t idx) const { return data[idx]; }

    auto AddLine(std::string_view line, char delimiter)
    {
        data.emplace_back(line | std::views::split(delimiter) | std::ranges::to<row_t>());
    }

    auto GetRow(size_t idx) const
    {
        if(idx >= data.size()) return std::vector<std::string>();
        return data[idx];
    }

    std::string GetJoinedRow(size_t idx, std::string_view sep = " ") const
    {
        if(idx >= data.size()) return "";
        return data[idx]
            | std::views::join_with(sep)
            | std::ranges::to<std::string>();
    }

    void Erase(size_t idx)
    {
        if(idx >= data.size()) return;
        data.erase(data.begin() + idx);
    }

    void Load(std::string_view filename, char delimiter)
    {
        data.clear();
        std::ifstream file(std::string{filename});
        if(!file)
        {
            return;
        }

        for(std::string line; std::getline(file,line);)
        {
            this->AddLine(line, delimiter);
        }
        return;
    }

    std::vector<std::string> GetMenuEntries(std::string_view viewTemplate) const
    {
        std::vector<std::string> entries;
        for(const row_t& row : data)
        {
            entries.emplace_back(substitute_template(viewTemplate, row));
        }
        return entries;
    }

    std::string Substitute(std::string_view strTemplate, size_t idx) const
    {
        if(idx >= data.size()) return "";
        return substitute_template(strTemplate, data[idx]);
    }
};
