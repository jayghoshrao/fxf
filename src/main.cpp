#include <iostream>

#include <CLI/CLI.hpp>
#include <ftxui/component/component.hpp>

#include "components.hpp"
#include "utils.hpp"
#include "read.hpp"
#include "registries.hpp"
#include "appstate.hpp"
#include "command.hpp"

using namespace ftxui;

int main(int argc, char* argv[]) {

    App& app = App::Instance();
    CommandRegistry::RegisterDefaultCommands();
    KeybindRegistry& keybinds = KeybindRegistry::Instance();
    KeybindRegistry::RegisterDefaultKeybinds();

    CLI::App args{"args"};

    std::string filename;
    args.add_option("file", filename, "file to read");
    args.add_option("-d,--delimiter", app.delimiter, "Delimiter");

    CLI11_PARSE(args, argc, argv);

    app.lines = io::read_lines(filename);
    app.menuEntries = app.lines;

    auto menu = gui::CreateMenu();
    auto status_bar = gui::CreateStatusBar();
    auto baseContainer = Container::Vertical({
            status_bar,
            menu,
            });

    auto commandDialog = gui::CreateCommandDialog();
    auto mainContainer = baseContainer | Modal(commandDialog, &app.commandDialog.isActive);
    auto mainEventHandler = CatchEvent(mainContainer, [&](Event event){
        if(app.commandDialog.isActive)
        {
            return false;
        }

        return keybinds.Execute(event);
    });


    menu->TakeFocus();
    app.screen.Loop(mainEventHandler);

    std::cout << app.debug << std::endl;

    return EXIT_SUCCESS;
}
