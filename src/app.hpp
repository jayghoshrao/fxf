#pragma once

#include <ftxui/component/screen_interactive.hpp>

#include "RowTable.hpp"

class App
{
public:
    struct ControlHandle {
        std::string string = "";
        bool isActive = false;
    };

    struct Controls {
        ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
        RowTable lines;
        char delimiter = '|';
        std::vector<std::string> menuEntries;
        int selector = 0;
        ControlHandle commandDialog;
        ControlHandle display;
        ControlHandle searchDialog;
        std::string debug = "";
        std::string viewTemplate = "{}";
    } controls;

    struct Cache {
        std::vector<std::string> menuEntries;
        RowTable lines;
    } cache;

public:
    static App& Instance() {
        static App instance;
        return instance;
    }

    void Load(const std::string& filename, char delimiter);
    void CreateGUI();
    void Loop();
    void ResetFocus();
    void FocusSearch();
    void ApplyViewTemplate(std::string_view viewTemplate);
    void ReapplyViewTemplate();

private:
    struct ComponentChildren {
        ftxui::Component menu{nullptr};
        ftxui::Component statusBar{nullptr};
        ftxui::Component baseContainer{nullptr};
        ftxui::Component mainContainer{nullptr};
        ftxui::Component commandDialog{nullptr};
        ftxui::Component mainEventHandler{nullptr};
        ftxui::Component searchInput{nullptr};
    } components;

    ftxui::Component CreateMenu();
    ftxui::Component CreateStatusBar();
    ftxui::Component CreateCommandDialog();
};
