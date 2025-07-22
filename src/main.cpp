#include <iostream>

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/string.hpp>

#include "read.hpp"

int main() {
    using namespace ftxui;

    auto table = io::read_table("test.csv");

    std::vector<Component> components;
    for(const auto& line : table[0])
    {
        components.push_back(Renderer([line] {
            return text(line);
        }));
    }

    bool isCheckboxTicked = false;
    auto checkbox = Checkbox("come check this", &isCheckboxTicked);

    auto main_container = Container::Vertical(components) | border;

    auto screen = ScreenInteractive::Fullscreen();
    auto app = CatchEvent(main_container, [&](Event event){
        if(event == Event::Character('c'))
        {
            isCheckboxTicked = !isCheckboxTicked;
            return true;
        }

        if(event == Event::Character('q'))
        {
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });


    screen.Loop(app);

    return EXIT_SUCCESS;
}
