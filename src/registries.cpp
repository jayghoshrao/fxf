#include "registries.hpp"
#include "app.hpp"

#include <filesystem>
#include <numeric>

bool CommandRegistry::Execute(const std::string& line) const {
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
    Register("read", [this](const std::vector<std::string>& args) {
        if(args.size() != 2)
            return false;

        std::string delimiter = args[0];

        const std::string& filename = args[1];
        if(!std::filesystem::is_regular_file(filename))
            return false;

        m_app.Load(filename, delimiter[0]);
        return true;
    });

    Register("quit", [this](const std::vector<std::string>& args) {
        m_app.screen.ExitLoopClosure()();
        return true;
    });

    Register("view", [this](const std::vector<std::string>& args) {
        if(args.size() < 1)
        {
            m_app.ApplyViewTemplate("{}");
            return true;
        }

        std::string viewTemplate;
        size_t join_start_idx = 0;
        for (size_t i = join_start_idx; i < args.size(); ++i) {
            if (i > join_start_idx) viewTemplate += " ";
            viewTemplate += args[i];
        }

        m_app.ApplyViewTemplate(viewTemplate);
        return true;
    });

    Register("bind", [this](const std::vector<std::string>& args){
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

        m_app.keybinds.Register(ftxui::Event::Character(key[0]), Command(cmdTemplate, Command::StringToExecutionPolicy(cmdType)));
        return true;
    });

    Register("delete", [this](const std::vector<std::string>& args) {
        m_app.state.lines.Erase(m_app.controls.selected);
        m_app.ReapplyViewTemplate();
        return true;
    });

    Register("command", [this](const std::vector<std::string>& args){
            if(args.size() < 3) return false;

            std::string name = args[0];
            std::string cmdType = args[1];
            std::string cmdTemplate;
            size_t join_start_idx = 2;
            for (size_t i = join_start_idx; i < args.size(); ++i) {
                if (i > join_start_idx) cmdTemplate += " ";
                cmdTemplate += args[i];
            }

            m_app.commands.Register(name, Command(cmdTemplate, Command::StringToExecutionPolicy(cmdType)));
            return true;
            });

    Register("cmd", Command(Get("command")));

    Register("show", [this](const std::vector<std::string>& args){
        if(args.size() < 1)
        {
            m_app.ApplyViewTemplate("{}");
            return true;
        }

        try {
            int idx = std::stoi(args[0]);
            m_app.ApplyViewTemplate("{" + args[0] +  "}");
            return true;
        } catch (const std::exception& e) {
            return false;
        }

        return false;
    });

    Register("open", [this](const std::vector<std::string>& args){
        auto str = m_app.state.lines.GetJoinedRow(m_app.controls.selected);
        if(const std::string& url = ExtractFirstURL(str); !url.empty())
        {
            Command("xdg-open" , Command::ExecutionPolicy::Silent).Execute(url);
            return true;
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

void KeybindRegistry::RegisterDefaultKeybinds()
{
    Register(
        ftxui::Event::q,
        Command("quit", Command::ExecutionPolicy::Alias)
    );

    Register(
        ftxui::Event::Character(':'),
        Command([this](const std::vector<std::string>&){
            m_app.controls.commandDialog.isActive = true;
            return true;
        })
    );

    Register(
        ftxui::Event::Character('/'),
        Command([this](const std::vector<std::string>&){
            m_app.cache.menuEntries = m_app.controls.menuEntries;
            m_app.cache.lines = m_app.state.lines;
            m_app.controls.searchDialog.placeholder = "Type to fuzzy search";
            m_app.FocusSearch();
            return true;
        })
    );

    auto vec0to9str = std::views::iota(0,10)
        | std::views::transform([](int i){return std::to_string(i);});
    for(const auto& numStr : vec0to9str)
    {
        Register(
            ftxui::Event::Character(numStr),
            Command("show " + numStr, Command::ExecutionPolicy::Alias)
        );
    }

    Register(
        ftxui::Event::Character('='),
        Command("show", Command::ExecutionPolicy::Alias)
    );

    Register(
        ftxui::Event::o,
        Command("open", Command::ExecutionPolicy::Alias)
    );
}
