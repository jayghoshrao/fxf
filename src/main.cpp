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

namespace state{
    static bool isCommandDialogShown = false;
    static Table table(0);
    static int selector = 0;
    static std::vector<std::string> menuEntries;
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
}

void ExecuteCommand(std::string commandString)
{

}

int main() {

    state::table = io::read_table("pocket.list");

    // state::menuEntries = state::table[0] | std::views::transform([](std::string_view line){
    //         return split_csv_line_view(line,'|')[0];
    //         }) | std::ranges::to<std::vector<std::string>>();

    state::menuEntries = state::table[0];

    auto menuOption = MenuOption();
    auto menu = Menu(&state::menuEntries, &state::selector, menuOption);

    std::string commandString = "";
    auto commandInputOption = InputOption::Default();
    commandInputOption.multiline = false;
    commandInputOption.on_enter = [&]{
        state::screen.Post([&]{

            auto result = split_csv_line_view(commandString, ' ');

            if(result[0] == "read"
                    && result.size() == 2 
                    && fs::is_regular_file(result[1]))
            {
                state::table = io::read_table(result[1]);
                state::menuEntries = state::table[0];
                state::selector = 0;
            }

            state::isCommandDialogShown = false;
            commandString = "";
            });
    };

    auto commandInput = Input(&commandString, &commandString, commandInputOption);
    commandInput |= CatchEvent([&](Event event){
            if(event == Event::Escape)
            {
                commandString = "";
                state::isCommandDialogShown = false;
                return true;
            }
            return false;
            });
    auto commandDialog = Renderer(commandInput, [&]{ return 
                commandInput->Render() | size(WIDTH, GREATER_THAN, 30)
            ;}) | border | center;

    auto mainContainer = Renderer(menu, [&]{ return menu->Render() | size(WIDTH, LESS_THAN, Terminal::Size().dimx - 3) | vscroll_indicator | frame | border;}) | Modal(commandDialog, &state::isCommandDialogShown);

    auto mainEventHandler = CatchEvent(mainContainer, [&](Event event){
        if(event == Event::Character('q'))
        {
            state::screen.ExitLoopClosure()();
            return true;
        }

        if(event == Event::Character(':'))
        {
            state::isCommandDialogShown = true;
            commandDialog->TakeFocus();
            return true;
        }

        return false;
    });


    mainContainer->TakeFocus();
    state::screen.Loop(mainEventHandler);

    return EXIT_SUCCESS;
}
