#include "registries.hpp"
#include "appstate.hpp"
#include "read.hpp"

#include <filesystem>

bool CommandRegistry::Execute(const std::string& line) const {
    AppState& appState = AppState::Instance();
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    if(auto it = commands_.find(cmd); it != commands_.end())
    {
        auto args = line
            | std::views::split(' ')
            | std::views::drop(1)
            | std::views::join_with(' ')
            | std::ranges::to<std::string>();

        return it->second.Execute(args);
    }
    return false;
}

void CommandRegistry::RegisterDefaultCommands()
{
    AppState& appState = AppState::Instance();
    CommandRegistry& commands = CommandRegistry::Instance();
    KeybindRegistry& keybinds = KeybindRegistry::Instance();

    commands.Register("read", [&](const std::vector<std::string>& args) {
        if(args.size() != 2)
            return false;

        std::string delimiter = args[0];

        const std::string& filename = args[1];
        if(!std::filesystem::is_regular_file(filename))
            return false;

        appState.lines = io::read_lines(filename);
        appState.menuEntries = appState.lines;
        appState.selector = 0; // TODO:
        return true;
    });

    commands.Register("quit", [&](const std::vector<std::string>& args) {
        appState.screen.ExitLoopClosure()();
        return true;
    });

    commands.Register("view", [&](const std::vector<std::string>& args) {
        if(args.size() < 1)
        {
            appState.menuEntries = appState.lines;
            return true;
        }

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
        if(args.size() < 3)
        {
            return false;
        }

        std::string key = args[0];
        std::string cmdType = args[1];
        std::string cmdTemplate;
        size_t join_start_idx = 2;
        for (size_t i = join_start_idx; i < args.size(); ++i) {
            if (i > join_start_idx) cmdTemplate += " ";
            cmdTemplate += args[i];
        }

        // keybinds.Register(key, Command(cmdTemplate, Command::StringToExecutionPolicy(cmdType)));
        keybinds.Register(ftxui::Event::Character(key[0]), Command(cmdTemplate, Command::StringToExecutionPolicy(cmdType)));
        return true;
    });

    commands.Register("delete", [&](const std::vector<std::string>& args) {
        appState.lines.erase(appState.lines.begin() + appState.selector);
        appState.menuEntries = appState.lines;
        return true;
    });

    commands.Register("command", [&](const std::vector<std::string>& args){
            if(args.size() < 3) return false;

            std::string name = args[0];
            std::string cmdType = args[1];
            std::string cmdTemplate;
            size_t join_start_idx = 2;
            for (size_t i = join_start_idx; i < args.size(); ++i) {
                if (i > join_start_idx) cmdTemplate += " ";
                cmdTemplate += args[i];
            }

            commands.Register(name, Command(cmdTemplate, Command::StringToExecutionPolicy(cmdType)));
            return true;
            });

    commands.Register("cmd", Command(commands.Get("command")));

}

// bool KeybindRegistry::Execute(std::string key) const{
bool KeybindRegistry::Execute(ftxui::Event event) const{
    if(auto it = map_.find(event); it != map_.end())
    {
        return it->second.Execute();
    }
    return false;
}

/*static*/ void KeybindRegistry::RegisterDefaultKeybinds()
{
    KeybindRegistry& keybinds = KeybindRegistry::Instance();
    keybinds.Register(
        ftxui::Event::q,
        Command("quit", Command::ExecutionPolicy::Alias)
    );

    keybinds.Register(
            ftxui::Event::Character(':'),
            Command([&](const std::vector<std::string>&){
                AppState& appState = AppState::Instance();
                appState.commandDialog.isActive = true;
                return true;
                })
            );

    keybinds.Register(
            ftxui::Event::Character('/'),
            Command([&](const std::vector<std::string>&){
                AppState& appState = AppState::Instance();
                appState.searchDialog.isActive = true;
                return true;
                })
            );
}
