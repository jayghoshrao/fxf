#include "appstate.hpp"
#include "components.hpp"
#include "registries.hpp"
#include "read.hpp"

using namespace ftxui;

void App::Load(const std::string& filename)
{
    controls.lines = io::read_lines(filename);
    controls.menuEntries = controls.lines;
}

void App::CreateGUI()
{
    components.menu = gui::CreateMenu();
    components.statusBar = gui::CreateStatusBar();
    components.baseContainer = Container::Vertical({
            components.statusBar,
            components.menu,
            });
    components.commandDialog = gui::CreateCommandDialog();

    components.mainContainer = components.baseContainer | Modal(components.commandDialog, &controls.commandDialog.isActive);

    CommandRegistry::RegisterDefaultCommands();
    KeybindRegistry::RegisterDefaultKeybinds();
    KeybindRegistry& keybinds = KeybindRegistry::Instance();

    components.mainEventHandler = CatchEvent(components.mainContainer, [&](Event event){
        if(controls.commandDialog.isActive)
        {
            return false;
        }

        return keybinds.Execute(event);
    });
}

void App::Loop()
{
    components.menu->TakeFocus();
    controls.screen.Loop(components.mainEventHandler);
}
