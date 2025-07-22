#include <iostream>

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/string.hpp>

#include "read.hpp"

int main() {
    using namespace ftxui;

    int selector{0};

    auto table = io::read_table("test.csv");

    auto menuopt = MenuOption();
    auto menu = Menu(&table[0], &selector, menuopt) | border;

    auto& main_container = menu;

    auto screen = ScreenInteractive::Fullscreen();
    auto app = CatchEvent(main_container, [&](Event event){
        if(event == Event::Character('q'))
        {
            screen.ExitLoopClosure()();
            return true;
        }

        return false;
    });


    main_container->TakeFocus();
    screen.Loop(app);

    return EXIT_SUCCESS;
}
