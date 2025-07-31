#pragma once

#include <ftxui/component/screen_interactive.hpp>

struct AppState
{
    struct Modal {
        std::string string = "";
        bool isShown = false;
    };

    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();

    std::vector<std::string> lines;
    char delimiter = '|';

    std::vector<std::string> menuEntries;
    int selector = 0;

    Modal commandDialog;
    Modal display;

    std::string debug = "";
};

AppState& GetAppState()
{
    static AppState instance;
    return instance;
}
