#include "components.hpp"
#include "appstate.hpp"
#include "registries.hpp"

namespace gui{

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
                    appState.commandDialog.isActive = false;
                    appState.commandDialog.string = "";
                    });
        };

        auto commandInput = Input(&appState.commandDialog.string, &appState.commandDialog.string, commandInputOption);
        commandInput |= CatchEvent([&](Event event){
                if(event == Event::Escape)
                {
                appState.commandDialog.string = "";
                appState.commandDialog.isActive = false;
                return true;
                }
                return false;
                });
        return Renderer(commandInput, [=]{ return
                commandInput->Render() | size(WIDTH, EQUAL, Terminal::Size().dimx * 0.5)
                ;}) | border ;
    }

    Component CreateMenu()
    {
        AppState& appState = AppState::Instance();
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

    Component CreateStatusBar()
    {
        AppState& appState = AppState::Instance();
        auto searchInputOption = InputOption::Default();
        searchInputOption.multiline = false;
        // searchInputOption.on_enter = [&]{
        //     appState.screen.Post([&]{
        //             // TODO: apply search, modify menuEntries, shift focus
        //             });
        // };

        auto searchInput = Input(&appState.searchDialog.string, &appState.searchDialog.string, searchInputOption) ;
        searchInput |= CatchEvent([&](Event event){
                if(event == Event::Escape)
                {
                appState.searchDialog.string = "";
                appState.searchDialog.isActive = false;
                // TODO: appState.ResetFocus()
                return true;
                }
                return false;
                });

        return searchInput | size(HEIGHT, EQUAL,1);

    }

}
