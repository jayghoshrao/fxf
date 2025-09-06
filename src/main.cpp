#include <iostream>
#include <CLI/CLI.hpp>

#include "app.hpp"

using namespace ftxui;

int main(int argc, char* argv[]) {

    App& app = App::Instance();
    CLI::App args{"args"};

    std::string filename;
    args.add_option("file", filename, "file to read");
    args.add_option("-d,--delimiter", app.controls.delimiter, "Delimiter");

    CLI11_PARSE(args, argc, argv);

    app.Load(filename);
    app.CreateGUI();
    app.Loop();

    std::cout << app.controls.debug << std::endl;

    return EXIT_SUCCESS;
}
