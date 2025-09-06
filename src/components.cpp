#include "components.hpp"
#include "appstate.hpp"
#include "registries.hpp"

namespace gui{

    using namespace ftxui;

    Component CreateCommandDialog()
    {
        App& app = App::Instance();
        CommandRegistry& commands = CommandRegistry::Instance();
        auto commandInputOption = InputOption::Default();
        commandInputOption.multiline = false;
        commandInputOption.on_enter = [&]{
            app.screen.Post([&]{
                    commands.Execute(app.commandDialog.string);
                    app.commandDialog.isActive = false;
                    app.commandDialog.string = "";
                    });
        };

        auto commandInput = Input(&app.commandDialog.string, &app.commandDialog.string, commandInputOption);
        commandInput |= CatchEvent([&](Event event){
                if(event == Event::Escape)
                {
                app.commandDialog.string = "";
                app.commandDialog.isActive = false;
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
        App& app = App::Instance();
        auto menuOption = MenuOption();
        auto menu = Menu(&app.menuEntries, &app.selector, menuOption)
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
        App& app = App::Instance();
        auto searchInputOption = InputOption::Default();
        searchInputOption.multiline = false;
        // searchInputOption.on_enter = [&]{
        //     app.screen.Post([&]{
        //             // TODO: apply search, modify menuEntries, shift focus
        //             });
        // };

        auto searchInput = Input(&app.searchDialog.string, &app.searchDialog.string, searchInputOption) ;
        searchInput |= CatchEvent([&](Event event){
                if(event == Event::Escape)
                {
                app.searchDialog.string = "";
                app.searchDialog.isActive = false;
                // TODO: app.ResetFocus()
                return true;
                }
                return false;
                });

        return searchInput | size(HEIGHT, EQUAL,1);

    }

}
