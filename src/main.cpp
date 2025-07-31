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
#include "command_registry.hpp"
#include "appstate.hpp"

using namespace ftxui;
namespace fs = std::filesystem;

class KeybindRegistry {
public:
    void Register(std::string event, std::string_view command) {
        map_[event] = command;
    }

    bool Execute(std::string key, AppState& appState) const {
        if(auto it = map_.find(key); it != map_.end())
        {
            auto currentSplit = split_csv_line(appState.lines[appState.selector], appState.delimiter);
            auto command = substitute_template(it->second, currentSplit);

            std::system(command.c_str());
            return true;
        }
        return false;
    }

private:
    std::map<std::string, std::string> map_;
};


Component CreateCommandDialog(AppState& appState, const CommandRegistry& registry)
{
    auto commandInputOption = InputOption::Default();
    commandInputOption.multiline = false;
    commandInputOption.on_enter = [&]{
        appState.screen.Post([&]{
            registry.Execute(appState.commandDialog.string);
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

    AppState& appState = GetAppState();
    CommandRegistry commands;
    KeybindRegistry keybinds;

    commands.Register("read", [&](const std::vector<std::string>& args) {
        if(args.size() != 2)
            return false;

        std::string delimiter = args[0];

        const std::string& filename = args[1];
        if(!fs::is_regular_file(filename))
            return false;

        appState.lines = io::read_lines(filename);
        appState.menuEntries = appState.lines;
        appState.selector = 0; // TODO:
        return true;
    });

    commands.Register("quit", [&](const std::vector<std::string>&) {
        appState.screen.ExitLoopClosure()();
        return true;
    });

    commands.Register("view", [&](const std::vector<std::string>& args) {
        if(args.size() < 1) 
        {
            appState.menuEntries = appState.lines;
            return true;
        }

        // std::string delimiter = args[1];
        std::string viewTemplate;
        size_t join_start_idx = 0;
        for (size_t i = join_start_idx; i < args.size(); ++i) {
            if (i > join_start_idx) viewTemplate += " ";
            viewTemplate += args[i];
        }

        size_t i=0;
        for(std::string& ref : appState.menuEntries)
        {
            auto currentSplit = split_csv_line(appState.lines[i++], appState.delimiter);
            ref = substitute_template(viewTemplate, currentSplit);
        }

        return true;
    });

    commands.Register("bind", [&](const std::vector<std::string>& args){
        if(args.size() < 2) 
        {
            return false;
        }

        std::string key = args[0];
        std::string cmdTemplate;
        size_t join_start_idx = 1;
        for (size_t i = join_start_idx; i < args.size(); ++i) {
            if (i > join_start_idx) cmdTemplate += " ";
            cmdTemplate += args[i];
        }

        keybinds.Register(key, cmdTemplate);
        return true;
    });

    commands.Register("delete", [&](const std::vector<std::string>& args) {
        appState.lines.erase(appState.lines.begin() + appState.selector); 
        appState.menuEntries = appState.lines;
        return true;
    });

    appState.lines = io::read_lines("local_bookmarks_youtube.txt");
    appState.menuEntries = appState.lines;

    auto menu = CreateMenu(appState);
    auto commandDialog = CreateCommandDialog(appState, commands);
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

        return keybinds.Execute(EventToString(event), appState);
    });


    mainContainer->TakeFocus();
    appState.screen.Loop(mainEventHandler);

    std::cout << appState.debug << std::endl;

    return EXIT_SUCCESS;
}
