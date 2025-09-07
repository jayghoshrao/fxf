#include "registries.hpp"
#include "app.hpp"

#include <filesystem>
#include <numeric>

bool CommandRegistry::Execute(const std::string& line) const {
    App& app = App::Instance();
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
    App& app = App::Instance();
    CommandRegistry& commands = CommandRegistry::Instance();
    KeybindRegistry& keybinds = KeybindRegistry::Instance();

    commands.Register("read", [&](const std::vector<std::string>& args) {
        if(args.size() != 2)
            return false;

        std::string delimiter = args[0];

        const std::string& filename = args[1];
        if(!std::filesystem::is_regular_file(filename))
            return false;

        app.Load(filename, delimiter[0]);
        return true;
    });

    commands.Register("quit", [&](const std::vector<std::string>& args) {
        app.controls.screen.ExitLoopClosure()();
        return true;
    });

    commands.Register("view", [&](const std::vector<std::string>& args) {
        if(args.size() < 1)
        {
            app.ApplyViewTemplate("{}");
            return true;
        }

        std::string viewTemplate;
        size_t join_start_idx = 0;
        for (size_t i = join_start_idx; i < args.size(); ++i) {
            if (i > join_start_idx) viewTemplate += " ";
            viewTemplate += args[i];
        }

        app.ApplyViewTemplate(viewTemplate);
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

        keybinds.Register(ftxui::Event::Character(key[0]), Command(cmdTemplate, Command::StringToExecutionPolicy(cmdType)));
        return true;
    });

    commands.Register("delete", [&](const std::vector<std::string>& args) {
        app.controls.lines.Erase(app.controls.selected);
        app.ReapplyViewTemplate();
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

    commands.Register("show", [&](const std::vector<std::string>& args){
        if(args.size() < 1)
        {
            app.ApplyViewTemplate("{}");
            return true;
        }

        try {
            int idx = std::stoi(args[0]);
            app.ApplyViewTemplate("{" + args[0] +  "}");
            return true;
        } catch (const std::exception& e) {
            return false;
        }

        return false;
    });

}

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
            App& app = App::Instance();
            app.controls.commandDialog.isActive = true;
            return true;
        })
    );

    keybinds.Register(
        ftxui::Event::Character('/'),
        Command([&](const std::vector<std::string>&){
            App& app = App::Instance();
            app.cache.menuEntries = app.controls.menuEntries;
            app.cache.lines = app.controls.lines;
            app.FocusSearch();
            return true;
        })
    );

    auto vec0to9str = std::views::iota(0,10) 
        | std::views::transform([&](int i){return std::to_string(i);});
    for(const auto& numStr : vec0to9str)
    {
        keybinds.Register(
            ftxui::Event::Character(numStr),
            Command("show " + numStr, Command::ExecutionPolicy::Alias)
        );
    }

    keybinds.Register(
        ftxui::Event::Character('='),
        Command("show", Command::ExecutionPolicy::Alias)
    );
}
