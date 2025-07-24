#pragma once
#include <string_view>
#include <vector>

std::vector<std::string> split_csv_line(std::string_view line, char delimiter = ',');
std::vector<std::string_view> split_csv_line_view(std::string_view line, char delimiter = ',');
