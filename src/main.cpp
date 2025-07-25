#include <iostream>

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/string.hpp>

#include "utils.hpp"
#include "read.hpp"

using namespace ftxui;

namespace state{
    static bool isCommandDialogShown = false;
}

int main() {
    int selector{0};

    auto table = io::read_table("local_bookmarks_youtube.txt");

    auto menuEntries = table[0] | std::views::transform([](std::string_view line){
            return split_csv_line_view(line,'|')[0];
            }) | std::ranges::to<std::vector<std::string>>();

    auto menuOption = MenuOption();
    auto menu = Menu(&menuEntries, &selector, menuOption);

    std::string commandString = "";
    auto commandInputOption = InputOption::Default();
    commandInputOption.multiline = false;
    commandInputOption.on_enter = [&]() {
        auto begin = commandString.find_first_not_of(" ");
        auto end = commandString.find_last_not_of(" ");
        auto str = commandString.substr(begin, end + 1 - begin);
        // f(str);
        state::isCommandDialogShown = false;
        commandString = "";
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

    auto mainContainer = Renderer(menu, [&]{ return menu->Render() | vscroll_indicator | frame | border;}) | Modal(commandDialog, &state::isCommandDialogShown);

    auto screen = ScreenInteractive::Fullscreen();
    auto app = CatchEvent(mainContainer, [&](Event event){
        if(event == Event::Character('q'))
        {
            screen.ExitLoopClosure()();
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
    screen.Loop(app);

    return EXIT_SUCCESS;
}
