#pragma once

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/box.hpp>

#include <optional>
#include <set>

#include "RowTable.hpp"
#include "registries.hpp"

class App
{
public:
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();

    struct ControlHandle {
        std::string placeholder = "";
        std::string string = "";
        int cursorPosition = 0;
        bool isActive = false;
    };

    struct State {
        RowTable lines;
        char delimiter = '|';
        std::string debug = "";
        std::string output = "";
    };

    struct Controls {
        std::vector<std::string> menuEntries;
        std::vector<size_t> filteredIndices;  // Maps display position -> original index
        std::set<size_t> selections;          // Selected original indices
        int selected = 0;
        int focused = 0;
        ControlHandle commandDialog;
        ControlHandle display;
        ControlHandle searchDialog;
        std::string viewTemplate = "{}";
        std::string searchPrompt = "> ";
    };

    struct Cache {
        std::vector<std::string> menuEntries;
    };

    struct ComponentChildren {
        ftxui::Component menu{nullptr};
        ftxui::Component statusBar{nullptr};
        ftxui::Component baseContainer{nullptr};
        ftxui::Component mainContainer{nullptr};
        ftxui::Component commandDialog{nullptr};
        ftxui::Component mainEventHandler{nullptr};
        ftxui::Component searchInput{nullptr};
        ftxui::Component searchPrompt{nullptr};
        ftxui::Box menuBox;
    }; 

public:
    static App& Instance() {
        static App instance;
        return instance;
    }

    CommandRegistry commands{*this};
    KeybindRegistry keybinds{*this};

    void Load(const std::string& filename, char delimiter);
    void CreateGUI();
    void Loop();
    void ResetFocus();
    void FocusSearch();
    void ApplyViewTemplate(std::string_view viewTemplate);
    void ReapplyViewTemplate();

    // Index and selection helpers
    std::optional<size_t> GetOriginalIndex(size_t displayIndex) const;
    bool IsSelected(size_t displayIndex) const;
    void ToggleSelection(size_t displayIndex);
    void ClearSelections();
    void SelectAll();
    void InvertSelections();
    void RefreshFilteredView();
    void ResetFilter();

private:
    ftxui::Component CreateMenu();
    ftxui::Component CreateStatusBar();
    ftxui::Component CreateCommandDialog();
    static bool HandleReadlineEvent(const ftxui::Event& event, std::string& str, int& cursor);

public:
    State state;
    Controls controls;
    Cache cache;

private:
    ComponentChildren components;

};
