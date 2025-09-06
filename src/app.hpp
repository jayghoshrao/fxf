#pragma once

#include <ftxui/component/screen_interactive.hpp>

class App
{
    public:
        struct ControlHandle {
            std::string string = "";
            bool isActive = false;
        };

        struct Controls {
            ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
            std::vector<std::string> lines;
            char delimiter = '|';
            std::vector<std::string> menuEntries;
            int selector = 0;
            ControlHandle commandDialog;
            ControlHandle display;
            ControlHandle searchDialog;
            std::string debug = "";
        } controls;

    public:
        static App& Instance() {
            static App instance;
            return instance;
        }

        void Load(const std::string& filename);
        void CreateGUI();
        void Loop();
        void ResetFocus();
        void FocusSearch();

    private:
        struct ComponentChildren {
            ftxui::Component menu{nullptr};
            ftxui::Component statusBar{nullptr};
            ftxui::Component baseContainer{nullptr};
            ftxui::Component mainContainer{nullptr};
            ftxui::Component commandDialog{nullptr};
            ftxui::Component mainEventHandler{nullptr};
        } components;

        ftxui::Component CreateMenu();
        ftxui::Component CreateStatusBar();
        ftxui::Component CreateCommandDialog();
};
