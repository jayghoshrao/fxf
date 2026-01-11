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
        auto maybeIdx = m_app.GetOriginalIndex(m_app.controls.selected);
        if (!maybeIdx) return false;
        size_t origIdx = *maybeIdx;

        // Remove from selections
        m_app.controls.selections.erase(origIdx);

        // Adjust selections: decrement indices greater than deleted
        std::set<size_t> adjusted;
        for (size_t idx : m_app.controls.selections) {
            if (idx > origIdx) {
                adjusted.insert(idx - 1);
            } else {
                adjusted.insert(idx);
            }
        }
        m_app.controls.selections = std::move(adjusted);

        // Remove from filteredIndices and adjust remaining indices
        auto& fi = m_app.controls.filteredIndices;
        fi.erase(std::remove(fi.begin(), fi.end(), origIdx), fi.end());
        for (size_t& idx : fi) {
            if (idx > origIdx) idx--;
        }

        // Delete from original data
        m_app.state.lines.Erase(origIdx);
        m_app.UpdateFilteredView();

        // Adjust selected if it's now out of bounds
        int maxIdx = static_cast<int>(fi.size()) - 1;
        if (m_app.controls.selected > maxIdx) {
            m_app.controls.selected = std::max(0, maxIdx);
        }

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
        auto maybeIdx = m_app.GetOriginalIndex(m_app.controls.selected);
        if (!maybeIdx) return false;
        auto str = m_app.state.lines.GetJoinedRow(*maybeIdx);
        if(const std::string& url = ExtractFirstURL(str); !url.empty())
        {
            Command("xdg-open" , Command::ExecutionPolicy::Silent).Execute(url);
            return true;
        }
        return false;
    });

    Register("select", [this](const std::vector<std::string>& args){
        if (m_app.controls.selections.empty()) {
            // No multi-selection: use current focused item
            auto maybeIdx = m_app.GetOriginalIndex(m_app.controls.selected);
            if (!maybeIdx) return false;
            m_app.state.output = m_app.state.lines.Substitute(m_app.controls.viewTemplate, *maybeIdx);
        } else {
            // Multi-selection: output in original data order
            std::string output;
            for (size_t origIdx = 0; origIdx < m_app.state.lines.data.size(); ++origIdx) {
                if (m_app.controls.selections.contains(origIdx)) {
                    if (!output.empty()) {
                        output += '\n';
                    }
                    output += m_app.state.lines.Substitute(m_app.controls.viewTemplate, origIdx);
                }
            }
            m_app.state.output = output;
        }
        m_app.screen.ExitLoopClosure()();
        return true;
    });

    Register("select-all", [this](const std::vector<std::string>& args) {
        m_app.SelectAll();
        return true;
    });

    Register("clear-selections", [this](const std::vector<std::string>& args) {
        m_app.ClearSelections();
        return true;
    });

    Register("invert-selections", [this](const std::vector<std::string>& args) {
        m_app.InvertSelections();
        return true;
    });

    Register("toggle-all", [this](const std::vector<std::string>& args) {
        m_app.InvertSelections();
        return true;
    });

    Register("preview", [this](const std::vector<std::string>& args) {
        m_app.TogglePreview();
        return true;
    });

    Register("preview-refresh", [this](const std::vector<std::string>& args) {
        m_app.scope.ClearCache();
        m_app.UpdatePreview();
        return true;
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
            m_app.cache.menuEntries = m_app.state.lines.GetMenuEntries(m_app.controls.viewTemplate);
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

    Register(
        ftxui::Event::Return,
        Command("select", Command::ExecutionPolicy::Alias)
    );

    // Tab: Toggle selection, stay in place
    Register(
        ftxui::Event::Tab,
        Command([this](const std::vector<std::string>&){
            m_app.ToggleSelection(m_app.controls.selected);
            return true;
        })
    );

    // Space: Toggle selection and move down
    Register(
        ftxui::Event::Character(' '),
        Command([this](const std::vector<std::string>&){
            m_app.ToggleSelection(m_app.controls.selected);
            if (m_app.controls.selected < static_cast<int>(m_app.controls.menuEntries.size()) - 1) {
                m_app.controls.selected++;
            }
            return true;
        })
    );

    // p: Toggle preview
    Register(
        ftxui::Event::Character('p'),
        Command("preview", Command::ExecutionPolicy::Alias)
    );

    // Shift+J: Scroll preview down
    Register(
        ftxui::Event::Character('J'),
        Command([this](const std::vector<std::string>&){
            if (m_app.controls.preview.isVisible) {
                m_app.ScrollPreviewDown();
            }
            return true;
        })
    );

    // Shift+K: Scroll preview up
    Register(
        ftxui::Event::Character('K'),
        Command([this](const std::vector<std::string>&){
            if (m_app.controls.preview.isVisible) {
                m_app.ScrollPreviewUp();
            }
            return true;
        })
    );

}
