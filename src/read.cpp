#include "read.hpp"
#include <fstream>

auto split_csv_line(std::string_view line, char delimiter = ',')
{
    return line
        | std::views::split(delimiter)
        | std::views::transform([](auto &&subrange)
                {
                return std::string_view(&*subrange.begin(), std::ranges::distance(subrange));
                });
}

namespace io{

    Table read_table(std::string_view filename)
    {
        std::ifstream file(std::string{filename});
        if(!file)
        {
            return Table(0);
        }

        std::string probeline;
        size_t size{0};
        if(std::getline(file, probeline))
        {
            size = std::ranges::distance(split_csv_line(probeline));
        }
        else
        {
            return Table(0);
        }

        Table table(size);

        if(size < 1) return table;

        file.clear();
        file.seekg(0, std::ios::beg);
        for(std::string line; std::getline(file,line);)
        {
            table.add_line(line);
        }
        return table;
    };

}
