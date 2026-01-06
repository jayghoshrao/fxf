#include "app.hpp"
#include "registries.hpp"
#include "utils.hpp"

#include <ftxui/component/component.hpp>
#include <numeric>

using namespace ftxui;

void App::Load(const std::string& filename, char delimiter)
{
    state.delimiter = delimiter;
    auto result = state.lines.Load(filename, state.delimiter);
    if (!result) {
        state.debug = result.error();
        return;
    }
    this->ApplyViewTemplate("{}");
    controls.selected = 0;
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

        // Half page scrolls
        if(event == Event::CtrlD) {
            int halfPage = (Terminal::Size().dimy - 2) / 2;  // -2 for status bar and divider
            int maxIdx = static_cast<int>(controls.menuEntries.size()) - 1;
            controls.selected = std::min(controls.selected + halfPage, maxIdx);
            return true;
        }
        if(event == Event::CtrlU) {
            int halfPage = (Terminal::Size().dimy - 2) / 2;
            controls.selected = std::max(controls.selected - halfPage, 0);
            return true;
        }

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

    commands.RegisterDefaultCommands();
    keybinds.RegisterDefaultKeybinds();

    components.mainEventHandler = CatchEvent(components.mainContainer, [this](Event event){
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
    controls.searchPrompt = "< ";
    components.menu->TakeFocus();
}

void App::FocusSearch()
{
    controls.commandDialog.isActive = false;
    controls.searchDialog.isActive = true;
    controls.searchPrompt = "> ";
    components.searchInput->TakeFocus();
}

bool App::HandleReadlineEvent(const Event& event, std::string& str, int& cursor)
{
    // Ensure cursor is within bounds
    cursor = std::clamp(cursor, 0, static_cast<int>(str.size()));

    // Ctrl+A: Move to beginning of line
    if (event == Event::Home || event.input() == std::string{1}) {  // Ctrl+A = ASCII 1
        cursor = 0;
        return true;
    }

    // Ctrl+E: Move to end of line
    if (event == Event::End || event.input() == std::string{5}) {  // Ctrl+E = ASCII 5
        cursor = static_cast<int>(str.size());
        return true;
    }

    // Ctrl+U: Delete from beginning to cursor
    if (event.input() == std::string{21}) {  // Ctrl+U = ASCII 21
        str.erase(0, cursor);
        cursor = 0;
        return true;
    }

    // Ctrl+K: Delete from cursor to end
    if (event.input() == std::string{11}) {  // Ctrl+K = ASCII 11
        str.erase(cursor);
        return true;
    }

    // Ctrl+W: Delete word before cursor
    if (event.input() == std::string{23}) {  // Ctrl+W = ASCII 23
        if (cursor > 0) {
            int end = cursor;
            // Skip trailing spaces
            while (cursor > 0 && str[cursor - 1] == ' ') {
                cursor--;
            }
            // Delete word characters
            while (cursor > 0 && str[cursor - 1] != ' ') {
                cursor--;
            }
            str.erase(cursor, end - cursor);
        }
        return true;
    }

    // Alt+B: Move back one word
    if (event == Event::AltB) {
        // Skip spaces
        while (cursor > 0 && str[cursor - 1] == ' ') {
            cursor--;
        }
        // Move to beginning of word
        while (cursor > 0 && str[cursor - 1] != ' ') {
            cursor--;
        }
        return true;
    }

    // Alt+F: Move forward one word
    if (event == Event::AltF) {
        int len = static_cast<int>(str.size());
        // Skip current word
        while (cursor < len && str[cursor] != ' ') {
            cursor++;
        }
        // Skip spaces
        while (cursor < len && str[cursor] == ' ') {
            cursor++;
        }
        return true;
    }

    // Ctrl+B: Move back one character (handled by FTXUI by default as ArrowLeft)
    // Ctrl+F: Move forward one character (handled by FTXUI by default as ArrowRight)

    return false;
}

Component App::CreateCommandDialog()
{
    auto commandInputOption = InputOption::Default();
    commandInputOption.multiline = false;
    commandInputOption.cursor_position = &controls.commandDialog.cursorPosition;
    commandInputOption.on_enter = [this]{
        screen.Post([this]{
            commands.Execute(controls.commandDialog.string);
            controls.commandDialog.isActive = false;
            controls.commandDialog.string = "";
            controls.commandDialog.cursorPosition = 0;
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
            controls.commandDialog.cursorPosition = 0;
            controls.commandDialog.isActive = false;
            return true;
        }
        return HandleReadlineEvent(event, controls.commandDialog.string, controls.commandDialog.cursorPosition);
    });
    return Renderer(commandInput, [=]{ return
        hbox({
            text(":"),
            commandInput->Render() | size(WIDTH, EQUAL, Terminal::Size().dimx * 0.5) });
    }) | border ;
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
    searchInputOption.cursor_position = &controls.searchDialog.cursorPosition;
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
            controls.searchDialog.cursorPosition = 0;
            controls.searchDialog.placeholder = "Press / to fuzzy search";
            ResetFocus();
            return true;
        }
        return HandleReadlineEvent(event, controls.searchDialog.string, controls.searchDialog.cursorPosition);
    });

    controls.searchPrompt = "< ";
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
