#pragma once

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/box.hpp>

#include <optional>
#include <set>
#include <future>
#include <atomic>

#include "RowTable.hpp"
#include "registries.hpp"
#include "scope.hpp"

// Application mode state machine
enum class AppMode {
    Normal,   // Menu navigation, keybinds active
    Search,   // Search input focused, live filtering
    Command   // Command dialog open (modal)
};

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

    struct PreviewState {
        bool isVisible = false;
        std::string content = "";
        size_t lastProcessedIndex = SIZE_MAX;
        int scrollPosition = 0;
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
        PreviewState preview;
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
        ftxui::Component previewPane{nullptr};
        ftxui::Box menuBox;
    }; 

public:
    static App& Instance() {
        static App instance;
        return instance;
    }

    CommandRegistry commands{*this};
    KeybindRegistry keybinds{*this};
    Scope scope;

    // Mode state machine
    AppMode mode = AppMode::Normal;
    void SetMode(AppMode newMode);
    AppMode GetMode() const { return mode; }

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
    void UpdateFilteredView();
    void RefreshFilteredView();
    void ResetFilter();
    void UpdateSearch();

    // Preview methods
    void TogglePreview();
    void UpdatePreview();
    void UpdatePreviewIfNeeded();
    void ScrollPreviewUp();
    void ScrollPreviewDown();

private:
    ftxui::Component CreateMenu();
    ftxui::Component CreateStatusBar();
    ftxui::Component CreateCommandDialog();
    ftxui::Component CreatePreviewPane();
    static bool HandleReadlineEvent(const ftxui::Event& event, std::string& str, int& cursor);

    // Async preview state
    std::future<std::string> m_previewFuture;
    std::atomic<size_t> m_previewRequestId{0};

    // Modal visibility for FTXUI Modal() operator
    bool m_commandModalVisible = false;

public:
    State state;
    Controls controls;
    Cache cache;

private:
    ComponentChildren components;

};
