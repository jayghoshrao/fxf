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

using namespace ftxui;
namespace fs = std::filesystem;

struct AppState
{
    std::vector<std::string> lines;
    std::vector<std::string> menuEntries;
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    int selector = 0;
    bool isCommandDialogShown = false;
    std::string commandString = "";
    char delimiter = '|';
};

std::string substitute_template(const std::string& template_str, const std::vector<std::string>& data) {
    std::string result = template_str;

    // Create joined string for {} placeholder
    std::string joined_data;
    if (!data.empty()) {
        joined_data = std::accumulate(data.begin() + 1, data.end(), data[0], 
            [](const std::string& acc, const std::string& s) {
                return acc + " | " + s;
            });
    }

    // Replace {} with joined data
    size_t pos = 0;
    while ((pos = result.find("{}", pos)) != std::string::npos) {
        result.replace(pos, 2, joined_data);
        pos += joined_data.length();
    }

    // Replace numbered placeholders {0}, {1}, {2}, etc.
    for (size_t i = 0; i < data.size(); ++i) {
        std::string placeholder = "{" + std::to_string(i) + "}";
        pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), data[i]);
            pos += data[i].length();
        }
    }

    return result;
}


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
    CommandRegistry commands;
    KeybindRegistry keybinds;

    commands.Register("read", [&](const std::vector<std::string>& args) {
        if(args.size() != 3)
            return false;

        std::string delimiter = args[1];

        const std::string& filename = args[2];
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
        if(args.size() < 2) 
        {
            appState.menuEntries = appState.lines;
            return true;
        }

        // std::string delimiter = args[1];
        std::string viewTemplate;
        size_t join_start_idx = 1;
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
        if(args.size() < 3) 
        {
            return false;
        }

        std::string key = args[1];
        std::string cmdTemplate;
        size_t join_start_idx = 2;
        for (size_t i = join_start_idx; i < args.size(); ++i) {
            if (i > join_start_idx) cmdTemplate += " ";
            cmdTemplate += args[i];
        }

        keybinds.Register(key, cmdTemplate);
        return true;
    });

    appState.lines = io::read_lines("local_bookmarks_youtube.txt");
    appState.menuEntries = appState.lines;

    auto menu = CreateMenu(appState);
    auto commandDialog = CreateCommandDialog(appState, commands);
    auto mainContainer = menu | Modal(commandDialog, &appState.isCommandDialogShown);
    auto mainEventHandler = CatchEvent(mainContainer, [&](Event event){
        if(appState.isCommandDialogShown)
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
            appState.isCommandDialogShown = true;
            return true;
        }

        return keybinds.Execute(EventToString(event), appState);
    });


    mainContainer->TakeFocus();
    appState.screen.Loop(mainEventHandler);

    return EXIT_SUCCESS;
}
