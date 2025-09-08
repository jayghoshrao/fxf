#include "app.hpp"
#include "registries.hpp"
#include "utils.hpp"

#include <ftxui/component/component.hpp>
#include <numeric>

using namespace ftxui;

void App::Load(const std::string& filename, char delimiter)
{
    state.delimiter = delimiter;
    state.lines.Load(filename, state.delimiter);
    this->ApplyViewTemplate("{}");
    controls.selected = 0; // TODO:
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
    screen.Loop(components.mainEventHandler);
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
        screen.Post([&]{
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
    auto menuEntryOption = MenuEntryOption();
    menuEntryOption.transform = [](const EntryState& s) {
        int scrollBarWidth = 1;
        int entryWidth = Terminal::Size().dimx - scrollBarWidth;
        int ellipsesWidth = 3;
        std::string label = s.label;
        if(label.size() > entryWidth) label = label.substr(0, entryWidth-ellipsesWidth) + "...";
        auto elem = text(label);
        if (s.active) elem = elem | bold | inverted;
        return elem;
    };

    auto menuOption = MenuOption();
    menuOption.entries_option = menuEntryOption;
    menuOption.focused_entry = &controls.focused;
    auto menu = Menu(&controls.menuEntries, &controls.selected, menuOption) | vscroll_indicator | frame;
    menu |= CatchEvent([&](Event event) {
        if(controls.selected == 0 
            && (event == Event::k || event == Event::ArrowUp))
        {
            return true;
        }

        if(controls.selected == controls.menuEntries.size() - 1 
            && (event == Event::j || event == Event::ArrowDown))
        {
            return true;
        }

        return false;
    });
    return menu;
}

Component App::CreateStatusBar()
{
    auto searchInputOption = InputOption::Default();
    searchInputOption.multiline = false;
    controls.searchDialog.placeholder = "Press / to fuzzy search";
    searchInputOption.on_enter = [&]{
        screen.Post([&]{
            controls.focused = 0;
            if(controls.searchDialog.string.empty())
            {
                controls.searchDialog.placeholder = "Press / to fuzzy search";
            }
            this->ResetFocus();
        });
    };

    searchInputOption.transform = [&](InputState state) {
        state.element |= color(Color::White);
        if(state.is_placeholder) { state.element |= dim; }
        return state.element;
    };

    searchInputOption.on_change = [&]{
        screen.Post([&]{
            auto fuzzyResults = extract(controls.searchDialog.string, cache.menuEntries, 0.0);
            std::vector<size_t> indices(fuzzyResults.size());
            std::ranges::iota(indices, 0);

            std::ranges::sort(indices, std::ranges::greater{}, [&](size_t i) {
                return fuzzyResults[i].second;
            });

            auto sortedLines = indices | std::views::transform([&](size_t i) { return cache.lines[i]; });

            // Copy back the reordered views into original vectors
            std::ranges::copy(sortedLines, state.lines.data.begin());
            controls.menuEntries = state.lines.GetMenuEntries(controls.viewTemplate);
            controls.selected = 0;
        });
    };


    auto searchInput = Input(&controls.searchDialog.string, &controls.searchDialog.placeholder, searchInputOption) ;
    searchInput |= CatchEvent([&](Event event){
        if(event == Event::Escape)
        {
            controls.searchDialog.string = "";
            controls.searchDialog.placeholder = "Press / to fuzzy search";
            ResetFocus();
            return true;
        }
        return false;
    });

    controls.searchPrompt = "> ";
    components.searchPrompt = Renderer([&]{
        return text(controls.searchPrompt);
    });

    auto currentViewTemplate = Renderer([&]{
        return text(controls.viewTemplate);
    });

    auto debug = Renderer([&]{
        auto divider = state.debug.empty() ? "" : " | "; 
        return text(divider + state.debug);
    });

    auto barTabs = Container::Horizontal({components.searchPrompt, searchInput, currentViewTemplate, debug}) | size(HEIGHT, EQUAL,1);

    components.searchInput = searchInput;
    return barTabs;
}

void App::ApplyViewTemplate(std::string_view viewTemplate)
{
    controls.viewTemplate = viewTemplate;
    controls.menuEntries = state.lines.GetMenuEntries(controls.viewTemplate);
}

void App::ReapplyViewTemplate()
{
    controls.menuEntries = state.lines.GetMenuEntries(controls.viewTemplate);
}
