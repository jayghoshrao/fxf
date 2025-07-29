#include <iostream>
#include <filesystem>

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/string.hpp>

#include "utils.hpp"
#include "read.hpp"
#include "command_registry.hpp"

using namespace ftxui;
namespace fs = std::filesystem;

struct AppState
{
    bool isCommandDialogShown = false;
    Table table{0};
    int selector = 0;
    std::vector<std::string> menuEntries;
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    std::string commandString = "";
};

Component CreateCommandDialog(AppState& appState, const CommandRegistry& registry)
{
    auto commandInputOption = InputOption::Default();
    commandInputOption.multiline = false;
    commandInputOption.on_enter = [&]{
        appState.screen.Post([&]{
            registry.Execute(appState.commandString);
            appState.isCommandDialogShown = false;
            appState.commandString = "";
        });
    };

    auto commandInput = Input(&appState.commandString, &appState.commandString, commandInputOption);
    commandInput |= CatchEvent([&](Event event){
        if(event == Event::Escape)
        {
            appState.commandString = "";
            appState.isCommandDialogShown = false;
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

    AppState appState;
    CommandRegistry registry;

    registry.Register("read", [&](const std::vector<std::string>& args) {
        if(args.size() != 2)
            return false;

        const std::string& filename = args[1];
        if(!fs::is_regular_file(filename))
            return false;

        appState.table = io::read_table(filename);
        appState.menuEntries = appState.table[0];
        appState.selector = 0; // TODO:
        return true;
    });

    registry.Register("quit", [&](const std::vector<std::string>&) {
        appState.screen.ExitLoopClosure()();
        return true;
    });

    appState.table = io::read_table("local_bookmarks_youtube.txt");

    // appState.menuEntries = appState.table[0] | std::views::transform([](std::string_view line){
    //         return split_csv_line_view(line,'|')[0];
    //         }) | std::ranges::to<std::vector<std::string>>();

    appState.menuEntries = appState.table[0];

    auto menu = CreateMenu(appState);

    auto commandDialog = CreateCommandDialog(appState, registry);

    auto mainContainer = menu | Modal(commandDialog, &appState.isCommandDialogShown);

    auto mainEventHandler = CatchEvent(mainContainer, [&](Event event){
        if(event == Event::Character('q') && appState.isCommandDialogShown != true)
        {
            appState.screen.ExitLoopClosure()();
            return true;
        }

        if(event == Event::Character(':'))
        {
            appState.isCommandDialogShown = true;
            return true;
        }

        return false;
    });


    mainContainer->TakeFocus();
    appState.screen.Loop(mainEventHandler);

    return EXIT_SUCCESS;
}
