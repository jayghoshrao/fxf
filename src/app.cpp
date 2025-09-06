#include "app.hpp"
#include "registries.hpp"
#include "utils.hpp"

#include <ftxui/component/component.hpp>

using namespace ftxui;

void App::Load(const std::string& filename, char delimiter)
{
    controls.delimiter = delimiter;
    controls.lines.Load(filename, controls.delimiter);
    this->ApplyViewTemplate("{}");
    controls.selector = 0; // TODO:
}

void App::CreateGUI()
{
    components.menu = this->CreateMenu();
    components.menu |= CatchEvent([&](Event event){
        if(event == Event::Character('G'))
        {
            components.menu->OnEvent(Event::End);
            return true;
        }
        static bool got_g = false;

        if(event == Event::Character('g')) {
            if(got_g) {
                got_g = false;
                components.menu->OnEvent(Event::Home);
                return true;
            } else {
                got_g = true;
                return true;
            }
        }

        got_g = false;
        return false;
    });

    components.statusBar = this->CreateStatusBar();
    components.baseContainer = Container::Vertical({
            components.statusBar,
            components.menu,
            });
    components.commandDialog = this->CreateCommandDialog();

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

void App::ResetFocus()
{
    controls.commandDialog.isActive = false;
    controls.searchDialog.isActive = false;
    components.menu->TakeFocus();
}

void App::FocusSearch()
{
    controls.commandDialog.isActive = false;
    controls.searchDialog.isActive = true;
    components.statusBar->TakeFocus();
}

Component App::CreateCommandDialog()
{
    CommandRegistry& commands = CommandRegistry::Instance();
    auto commandInputOption = InputOption::Default();
    commandInputOption.multiline = false;
    commandInputOption.on_enter = [&]{
        controls.screen.Post([&]{
            commands.Execute(controls.commandDialog.string);
            controls.commandDialog.isActive = false;
            controls.commandDialog.string = "";
        });
    };

    auto commandInput = Input(&controls.commandDialog.string, &controls.commandDialog.string, commandInputOption);
    commandInput |= CatchEvent([&](Event event){
        if(event == Event::Escape)
        {
            controls.commandDialog.string = "";
            controls.commandDialog.isActive = false;
            return true;
        }
        return false;
    });
    return Renderer(commandInput, [=]{ return
        commandInput->Render() | size(WIDTH, EQUAL, Terminal::Size().dimx * 0.5)
        ;}) | border ;
}

Component App::CreateMenu()
{
    auto menuOption = MenuOption();
    auto menu = Menu(&controls.menuEntries, &controls.selector, menuOption)
        | vscroll_indicator | frame | border;
    return menu;
}

Component App::CreateStatusBar()
{
    auto searchInputOption = InputOption::Default();
    searchInputOption.multiline = false;
    searchInputOption.on_enter = [&]{
        controls.screen.Post([&]{
            this->ResetFocus();
        });
    };

    searchInputOption.on_change = [&]{
        controls.screen.Post([&]{
            auto fuzzyResults = extract(controls.searchDialog.string, cache.menuEntries, 0.0);
            std::ranges::sort(fuzzyResults, std::ranges::greater{}, &std::pair<std::string, double>::second);

            controls.menuEntries.clear();
            for(const auto& item : fuzzyResults)
            {
                controls.menuEntries.emplace_back(item.first);
            }
        });
    };


    auto searchInput = Input(&controls.searchDialog.string, &controls.searchDialog.string, searchInputOption) ;
    searchInput |= CatchEvent([&](Event event){
        if(event == Event::Escape)
        {
            controls.searchDialog.string = "";
            ResetFocus();
            return true;
        }
        return false;
    });

    return searchInput | size(HEIGHT, EQUAL,1);
}

void App::ApplyViewTemplate(std::string_view viewTemplate)
{
    controls.viewTemplate = viewTemplate;
    controls.menuEntries = controls.lines.GetMenuEntries(controls.viewTemplate);
}

void App::ReapplyViewTemplate()
{
    controls.menuEntries = controls.lines.GetMenuEntries(controls.viewTemplate);
}
