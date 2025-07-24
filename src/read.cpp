#include "read.hpp"
#include <fstream>

#include "utils.hpp"

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
            size = split_csv_line_view(probeline).size();
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
