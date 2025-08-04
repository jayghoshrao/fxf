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

#include "utils.hpp"
#include "read.hpp"
#include "registries.hpp"
#include "appstate.hpp"
#include "command.hpp"

using namespace ftxui;

Component CreateCommandDialog()
{
    AppState& appState = AppState::Instance();
    CommandRegistry& commands = CommandRegistry::Instance();
    auto commandInputOption = InputOption::Default();
    commandInputOption.multiline = false;
    commandInputOption.on_enter = [&]{
        appState.screen.Post([&]{
            commands.Execute(appState.commandDialog.string);
            appState.commandDialog.isShown = false;
            appState.commandDialog.string = "";
        });
    };

    auto commandInput = Input(&appState.commandDialog.string, &appState.commandDialog.string, commandInputOption);
    commandInput |= CatchEvent([&](Event event){
        if(event == Event::Escape)
        {
            appState.commandDialog.string = "";
            appState.commandDialog.isShown = false;
            return true;
        }
        return false;
    });
    return Renderer(commandInput, [=]{ return 
        commandInput->Render() | size(WIDTH, EQUAL, Terminal::Size().dimx * 0.5)
        ;}) | border ;
}

Component CreateMenu(AppState& appState)
{
    auto menuOption = MenuOption();
    auto menu = Menu(&appState.menuEntries, &appState.selector, menuOption)
        | vscroll_indicator | frame | border;

    menu |= CatchEvent([&](Event event){
        if(event == Event::Character('G'))
        {
            menu->OnEvent(Event::End);
            return true;
        }
        static bool got_g = false;

        if(event == Event::Character('g')) {
            if(got_g) {
                got_g = false;
                menu->OnEvent(Event::Home);
                return true;
            } else {
                got_g = true;
                return true;
            }
        }

        got_g = false;
        return false;
    });

    return menu;
}



int main() {

    AppState& appState = AppState::Instance();
    KeybindRegistry& keybinds = KeybindRegistry::Instance();

    RegisterDefaultCommands();

    appState.lines = io::read_lines("local_bookmarks_youtube.txt");
    appState.menuEntries = appState.lines;

    auto menu = CreateMenu(appState);
    auto commandDialog = CreateCommandDialog();
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
