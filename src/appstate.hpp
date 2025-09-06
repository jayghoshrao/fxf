#pragma once

#include <ftxui/component/screen_interactive.hpp>

struct App
{
    static App& Instance() {
        static App instance;
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
