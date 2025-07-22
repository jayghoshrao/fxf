#include <iostream>

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/string.hpp>

int main() {
    using namespace ftxui;

    bool isCheckboxTicked = false;
    auto checkbox = Checkbox("come check this", &isCheckboxTicked);

    auto main_container = Container::Vertical({checkbox});

    auto app = CatchEvent(main_container | border, [&](Event event){
        if(event == Event::Character('c'))
        {
            isCheckboxTicked = !isCheckboxTicked;
            return true;
        }
        return false;
    });

    auto screen = ScreenInteractive::Fullscreen();

    screen.Loop(app);

    return EXIT_SUCCESS;
}
