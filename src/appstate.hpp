#pragma once

#include <ftxui/component/screen_interactive.hpp>

struct AppState
{
    static AppState& Instance() {
        static AppState instance;
        return instance;
    }

    struct FocusableInput {
        std::string string = "";
        bool isActive = false;
    };

    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();

    std::vector<std::string> lines;
    char delimiter = '|';

    std::vector<std::string> menuEntries;
    int selector = 0;

    FocusableInput commandDialog;
    FocusableInput display;
    FocusableInput searchDialog;

    std::string debug = "";
};
