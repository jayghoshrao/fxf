#include <iostream>
#include <filesystem>

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/string.hpp>

#include "utils.hpp"
#include "read.hpp"

using namespace ftxui;
namespace fs = std::filesystem;

struct AppState
{
    bool isCommandDialogShown = false;
    Table table{0};
    int selector = 0;
    std::vector<std::string> menuEntries;
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
};

int main() {

    AppState appState;
    appState.table = io::read_table("pocket.list");

    // appState.menuEntries = appState.table[0] | std::views::transform([](std::string_view line){
    //         return split_csv_line_view(line,'|')[0];
    //         }) | std::ranges::to<std::vector<std::string>>();

    appState.menuEntries = appState.table[0];

    auto menuOption = MenuOption();
    auto menu = Menu(&appState.menuEntries, &appState.selector, menuOption);

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

    std::string commandString = "";
    auto commandInputOption = InputOption::Default();
    commandInputOption.multiline = false;
    commandInputOption.on_enter = [&]{
        appState.screen.Post([&]{

            auto result = split_csv_line_view(commandString, ' ');

            if(result[0] == "read"
                && result.size() == 2 
                && fs::is_regular_file(result[1]))
            {
                appState.table = io::read_table(result[1]);
                appState.menuEntries = appState.table[0];
                appState.selector = 0;
            }

            appState.isCommandDialogShown = false;
            commandString = "";
        });
    };

    auto commandInput = Input(&commandString, &commandString, commandInputOption);
    commandInput |= CatchEvent([&](Event event){
        if(event == Event::Escape)
        {
            commandString = "";
            appState.isCommandDialogShown = false;
            return true;
        }
        return false;
    });
    auto commandDialog = Renderer(commandInput, [&]{ return 
        commandInput->Render() | size(WIDTH, GREATER_THAN, 30)
        ;}) | border | center;

    auto mainContainer = Renderer(menu, [&]{ return menu->Render() | size(WIDTH, LESS_THAN, Terminal::Size().dimx - 3) | vscroll_indicator | frame | border;}) | Modal(commandDialog, &appState.isCommandDialogShown);

    auto mainEventHandler = CatchEvent(mainContainer, [&](Event event){
        if(event == Event::Character('q'))
        {
            appState.screen.ExitLoopClosure()();
            return true;
        }

        if(event == Event::Character(':'))
        {
            appState.isCommandDialogShown = true;
            commandDialog->TakeFocus();
            return true;
        }

        return false;
    });


    mainContainer->TakeFocus();
    appState.screen.Loop(mainEventHandler);

    return EXIT_SUCCESS;
}
