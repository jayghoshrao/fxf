#include "read.hpp"
#include <fstream>

#include "utils.hpp"

namespace io{

    std::vector<std::string> read_lines(std::string filename){
        std::vector<std::string> lines;
        std::ifstream file(filename);

        if(file)
        {
            for(std::string line; std::getline(file,line);)
            {
                lines.emplace_back(line);
            }
        }

        return lines;
    }

    Table read_table(std::string_view filename, char delimiter /*= ','*/)
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
            size = split_csv_line_view(probeline, delimiter).size();
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
            table.add_line(line, delimiter);
        }
        return table;
    };


}
