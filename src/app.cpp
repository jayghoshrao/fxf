#include "app.hpp"
#include "registries.hpp"
#include "utils.hpp"

#include <ftxui/component/component.hpp>
#include <numeric>

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

        if(event == Event::CtrlF) { return components.menu->OnEvent(Event::PageDown); }
        if(event == Event::CtrlB) { return components.menu->OnEvent(Event::PageUp); }

        // TODO: half page scrolls
        if(event == Event::CtrlD) { return components.menu->OnEvent(Event::PageDown); }
        if(event == Event::CtrlU) { return components.menu->OnEvent(Event::PageUp); }

        got_g = false;
        return false;
    });

    auto divider = Renderer([&]{
        return separatorHeavy();
    }) | size(HEIGHT, EQUAL, 1);

    components.statusBar = this->CreateStatusBar();
    components.baseContainer = Container::Vertical({
            components.statusBar,
            divider,
            components.menu,
            });
    components.commandDialog = this->CreateCommandDialog();

    components.mainContainer = components.baseContainer | Modal(components.commandDialog, &controls.commandDialog.isActive);

    CommandRegistry::RegisterDefaultCommands();
    KeybindRegistry::RegisterDefaultKeybinds();
    KeybindRegistry& keybinds = KeybindRegistry::Instance();

    components.mainEventHandler = CatchEvent(components.mainContainer, [&](Event event){
        if(controls.commandDialog.isActive || controls.searchDialog.isActive)
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
    components.searchInput->TakeFocus();
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

    commandInputOption.transform = [&](InputState state) {
        state.element |= color(Color::White);

        if (state.is_placeholder) {
            state.element |= dim;
        }

        return state.element;
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
        | vscroll_indicator | frame;
    return menu;
}

Component App::CreateStatusBar()
{
    auto searchInputOption = InputOption::Default();
    searchInputOption.multiline = false;
    searchInputOption.placeholder = "Press / to fuzzy search";
    searchInputOption.on_enter = [&]{
        controls.screen.Post([&]{
            this->ResetFocus();
        });
    };

    searchInputOption.transform = [&](InputState state) {

        state.element |= color(Color::White);

        if (state.is_placeholder) {
            state.element |= dim;
        }

        return state.element;
    };


    searchInputOption.on_change = [&]{
        controls.screen.Post([&]{
            auto fuzzyResults = extract(controls.searchDialog.string, cache.menuEntries, 0.0);
            std::vector<size_t> indices(fuzzyResults.size());
            std::ranges::iota(indices, 0);

            std::ranges::sort(indices, std::ranges::greater{}, [&](size_t i) {
                return fuzzyResults[i].second;
            });

            auto sortedLines = indices | std::views::transform([&](size_t i) { return cache.lines[i]; });

            // Copy back the reordered views into original vectors
            std::ranges::copy(sortedLines, controls.lines.data.begin());
            controls.menuEntries = controls.lines.GetMenuEntries(controls.viewTemplate);
            controls.selector = 0;
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

    controls.searchPrompt = "> ";
    components.searchPrompt = Renderer([&]{
        return text(controls.searchPrompt);
    });

    auto barTabs = Container::Horizontal({components.searchPrompt, searchInput}) | size(HEIGHT, EQUAL,1);

    components.searchInput = searchInput;
    return barTabs;
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
