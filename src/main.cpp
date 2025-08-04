#include <iostream>
#include <filesystem>
#include <numeric>
#include <cstdlib>
#include <map>


#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/string.hpp>

#include "components.hpp"
#include "utils.hpp"
#include "read.hpp"
#include "registries.hpp"
#include "appstate.hpp"
#include "command.hpp"

using namespace ftxui;



int main() {

    AppState& appState = AppState::Instance();
    KeybindRegistry& keybinds = KeybindRegistry::Instance();

    RegisterDefaultCommands();

    appState.lines = io::read_lines("local_bookmarks_youtube.txt");
    appState.menuEntries = appState.lines;

    auto menu = gui::CreateMenu();
    auto commandDialog = gui::CreateCommandDialog();
    auto mainContainer = menu | Modal(commandDialog, &appState.commandDialog.isShown);
    auto mainEventHandler = CatchEvent(mainContainer, [&](Event event){
        if(appState.commandDialog.isShown)
        {
            return false;
        }

        if(event == Event::Character('q'))
        {
            appState.screen.ExitLoopClosure()();
            return true;
        }

        if(event == Event::Character(':'))
        {
            appState.commandDialog.isShown = true;
            return true;
        }

        return keybinds.Execute(EventToString(event));
    });


    mainContainer->TakeFocus();
    appState.screen.Loop(mainEventHandler);

    std::cout << appState.debug << std::endl;

    return EXIT_SUCCESS;
}
