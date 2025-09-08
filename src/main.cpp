#include <iostream>
#include <CLI/CLI.hpp>

#include "app.hpp"

using namespace ftxui;

int main(int argc, char* argv[]) {

    App& app = App::Instance();
    CLI::App args{"args"};

    std::string filename;
    char delimiter = '|';
    args.add_option("file", filename, "file to read");
    args.add_option("-d,--delimiter", delimiter, "Delimiter");

    CLI11_PARSE(args, argc, argv);

    app.CreateGUI();
    app.Load(filename, delimiter);
    app.Loop();

    std::cout << app.state.debug << std::endl;

    return EXIT_SUCCESS;
}
