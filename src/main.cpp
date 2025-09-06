#include <iostream>

#include <CLI/CLI.hpp>

#include "components.hpp"
#include "utils.hpp"
#include "read.hpp"
#include "registries.hpp"
#include "appstate.hpp"
#include "command.hpp"

using namespace ftxui;

int main(int argc, char* argv[]) {

    AppState& appState = AppState::Instance();
    CommandRegistry::RegisterDefaultCommands();
    KeybindRegistry& keybinds = KeybindRegistry::Instance();
    KeybindRegistry::RegisterDefaultKeybinds();

    CLI::App args{"args"};

    std::string filename;
    args.add_option("file", filename, "file to read");
    args.add_option("-d,--delimiter", appState.delimiter, "Delimiter");

    CLI11_PARSE(args, argc, argv);

    appState.lines = io::read_lines(filename);
    appState.menuEntries = appState.lines;

    auto menu = gui::CreateMenu();
    auto commandDialog = gui::CreateCommandDialog();
    auto mainContainer = menu | Modal(commandDialog, &appState.commandDialog.isShown);
    auto mainEventHandler = CatchEvent(mainContainer, [&](Event event){
        if(appState.commandDialog.isShown)
        {
            return false;
        }

        return keybinds.Execute(event);
    });


    mainContainer->TakeFocus();
    appState.screen.Loop(mainEventHandler);

    std::cout << appState.debug << std::endl;

    return EXIT_SUCCESS;
}
