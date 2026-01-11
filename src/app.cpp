#include "app.hpp"
#include "registries.hpp"
#include "utils.hpp"

#include <ftxui/component/component.hpp>
#include <numeric>
#include <sstream>

using namespace ftxui;

void App::Load(const std::string& filename, char delimiter)
{
    state.delimiter = delimiter;
    auto result = state.lines.Load(filename, state.delimiter);
    if (!result) {
        state.debug = result.error();
        return;
    }
    controls.viewTemplate = "{}";
    ResetFilter();
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
            int menuHeight = components.menuBox.y_max - components.menuBox.y_min + 1;
            int halfPage = std::max(1, menuHeight / 2);
            int maxIdx = static_cast<int>(controls.menuEntries.size()) - 1;
            controls.selected = std::min(controls.selected + halfPage, maxIdx);
            return true;
        }
        if(event == Event::CtrlU) {
            int menuHeight = components.menuBox.y_max - components.menuBox.y_min + 1;
            int halfPage = std::max(1, menuHeight / 2);
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
    components.previewPane = this->CreatePreviewPane();

    // Content area: menu with optional preview split
    // Use Renderer(child, render_fn) to keep menu in component tree for focus/events
    auto menuWithPreview = Renderer(components.menu, [this]{
        int termWidth = Terminal::Size().dimx;
        int halfWidth = termWidth / 2;

        if (controls.preview.isVisible) {
            return hbox({
                components.menu->Render() | size(WIDTH, EQUAL, halfWidth),
                components.previewPane->Render() | size(WIDTH, EQUAL, halfWidth),
            });
        }
        return components.menu->Render() | flex;
    });

    components.baseContainer = Container::Vertical({
            components.statusBar,
            divider,
            menuWithPreview,
            });
    components.commandDialog = this->CreateCommandDialog();

    components.mainContainer = components.baseContainer | Modal(components.commandDialog, &controls.commandDialog.isActive);

    commands.RegisterDefaultCommands();
    keybinds.RegisterDefaultKeybinds();

    components.mainEventHandler = CatchEvent(components.mainContainer, [this](Event event){
        // Check actual focus state instead of manual flags for searchInput
        if(controls.commandDialog.isActive || components.searchInput->Focused())
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
    menuEntryOption.transform = [this](const EntryState& s) {
        int scrollBarWidth = 1;
        int termWidth = Terminal::Size().dimx;
        int menuWidth = controls.preview.isVisible ? termWidth / 2 : termWidth;
        int entryWidth = menuWidth - scrollBarWidth;
        int ellipsesWidth = 3;
        std::string label = s.label;
        if(static_cast<int>(label.size()) > entryWidth)
            label = label.substr(0, entryWidth - ellipsesWidth) + "...";

        bool isMultiSelected = (s.index >= 0) && IsSelected(static_cast<size_t>(s.index));

        auto elem = text(label);

        if (s.active && isMultiSelected) {
            elem = elem | bold | inverted | color(Color::Yellow);
        } else if (s.active) {
            elem = elem | bold | inverted;
        } else if (isMultiSelected) {
            elem = elem | bold | color(Color::Yellow);
        }

        return elem;
    };

    auto menuOption = MenuOption();
    menuOption.entries_option = menuEntryOption;
    menuOption.focused_entry = &controls.focused;
    menuOption.on_change = [this]{ UpdatePreviewIfNeeded(); };
    auto menu = Menu(&controls.menuEntries, &controls.selected, menuOption) | vscroll_indicator | yframe | reflect(components.menuBox);
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
    searchInputOption.on_enter = [this]{
        screen.Post([this]{
            controls.focused = 0;
            if(controls.searchDialog.string.empty())
            {
                controls.searchDialog.placeholder = "Press / to fuzzy search";
            }
            this->ResetFocus();
            this->UpdatePreviewIfNeeded();
        });
    };

    searchInputOption.transform = [&](InputState state) {
        state.element |= color(Color::White);
        if(state.is_placeholder) { state.element |= dim; }
        return state.element;
    };

    searchInputOption.on_change = [&]{ UpdateSearch(); };

    auto searchInput = Input(&controls.searchDialog.string, &controls.searchDialog.placeholder, searchInputOption) ;
    searchInput |= CatchEvent([this](Event event){
        if(event == Event::Escape)
        {
            controls.searchDialog.string = "";
            controls.searchDialog.cursorPosition = 0;
            controls.searchDialog.placeholder = "Press / to fuzzy search";
            ResetFilter();
            ResetFocus();
            UpdatePreviewIfNeeded();
            return true;
        }
        bool handled = HandleReadlineEvent(event, controls.searchDialog.string, controls.searchDialog.cursorPosition);
        if (handled) {
            UpdateSearch();
        }
        return handled;
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
    UpdateFilteredView();
}

void App::ReapplyViewTemplate()
{
    UpdateFilteredView();
}

std::optional<size_t> App::GetOriginalIndex(size_t displayIndex) const
{
    if (displayIndex >= controls.filteredIndices.size()) return std::nullopt;
    return controls.filteredIndices[displayIndex];
}

bool App::IsSelected(size_t displayIndex) const
{
    auto origIdx = GetOriginalIndex(displayIndex);
    if (!origIdx) return false;
    return controls.selections.contains(*origIdx);
}

void App::ToggleSelection(size_t displayIndex)
{
    auto origIdx = GetOriginalIndex(displayIndex);
    if (!origIdx) return;
    if (controls.selections.contains(*origIdx)) {
        controls.selections.erase(*origIdx);
    } else {
        controls.selections.insert(*origIdx);
    }
}

void App::ClearSelections()
{
    controls.selections.clear();
}

void App::SelectAll()
{
    for (size_t origIdx : controls.filteredIndices) {
        controls.selections.insert(origIdx);
    }
}

void App::InvertSelections()
{
    for (size_t origIdx : controls.filteredIndices) {
        if (controls.selections.contains(origIdx)) {
            controls.selections.erase(origIdx);
        } else {
            controls.selections.insert(origIdx);
        }
    }
}

void App::UpdateFilteredView()
{
    controls.menuEntries.clear();
    controls.menuEntries.reserve(controls.filteredIndices.size());
    for (size_t origIdx : controls.filteredIndices) {
        controls.menuEntries.push_back(
            substitute_template(controls.viewTemplate, state.lines[origIdx])
        );
    }
}

void App::RefreshFilteredView()
{
    controls.menuEntries.clear();
    controls.menuEntries.reserve(controls.filteredIndices.size());
    // TODO: perhaps just copy + re-sort?
    for (size_t origIdx : controls.filteredIndices) {
        controls.menuEntries.push_back(
            cache.menuEntries[origIdx]
        );
    }
}

void App::ResetFilter()
{
    controls.filteredIndices.clear();
    controls.filteredIndices.reserve(state.lines.data.size());
    for (size_t i = 0; i < state.lines.data.size(); ++i) {
        controls.filteredIndices.push_back(i);
    }
    UpdateFilteredView();
}

void App::UpdateSearch()
{
    screen.Post([&]{
        if (controls.searchDialog.string.empty()) {
            ResetFilter();
            controls.selected = 0;
            return;
        }

        auto fuzzyResults = extract(controls.searchDialog.string, cache.menuEntries);

        // Sort by score descending
        std::ranges::sort(fuzzyResults, std::ranges::greater{}, [](const auto& p) {
            return p.second;
        });

        // Extract indices in sorted order
        controls.filteredIndices.clear();
        controls.filteredIndices.reserve(fuzzyResults.size());
        for (const auto& [idx, score] : fuzzyResults) {
            controls.filteredIndices.push_back(idx);
        }
        RefreshFilteredView();
        controls.selected = 0;
    });
}

void App::TogglePreview()
{
    controls.preview.isVisible = !controls.preview.isVisible;
    if (controls.preview.isVisible) {
        UpdatePreview();
    }
}

void App::UpdatePreview()
{
    if (!controls.preview.isVisible) return;

    controls.preview.content = "Loading...";
    controls.preview.scrollPosition = 0;

    auto maybeIdx = GetOriginalIndex(static_cast<size_t>(controls.selected));
    if (!maybeIdx) {
        controls.preview.content = "No item selected";
        return;
    }

    std::string entry = state.lines.GetJoinedRow(*maybeIdx);
    size_t requestId = ++m_previewRequestId;

    m_previewFuture = std::async(std::launch::async, [this, entry, requestId]() -> std::string {
        std::string result = scope.Process(entry);
        screen.Post([this, result, requestId]{
            if (requestId == m_previewRequestId) {
                controls.preview.content = result;
            }
        });
        // Trigger a screen redraw after content is updated
        screen.PostEvent(Event::Custom);
        return result;
    });

    controls.preview.lastProcessedIndex = *maybeIdx;
}

void App::UpdatePreviewIfNeeded()
{
    if (!controls.preview.isVisible) return;

    auto maybeIdx = GetOriginalIndex(static_cast<size_t>(controls.selected));
    if (!maybeIdx) return;

    if (*maybeIdx != controls.preview.lastProcessedIndex) {
        UpdatePreview();
    }
}

void App::ScrollPreviewUp()
{
    if (controls.preview.scrollPosition > 0) {
        controls.preview.scrollPosition--;
    }
}

void App::ScrollPreviewDown()
{
    controls.preview.scrollPosition++;
}

Component App::CreatePreviewPane()
{
    return Renderer([this]{
        if (controls.preview.content.empty()) {
            return window(text(" Preview "), text("No preview") | dim | center) | flex;
        }

        Elements lines;
        std::istringstream iss(controls.preview.content);
        std::string line;
        while (std::getline(iss, line)) {
            lines.push_back(text(line));
        }

        // Apply scroll offset
        int scrollOffset = controls.preview.scrollPosition;
        int totalLines = static_cast<int>(lines.size());
        int visibleStart = std::min(scrollOffset, std::max(0, totalLines - 1));

        Elements visibleLines;
        for (int i = visibleStart; i < totalLines; ++i) {
            visibleLines.push_back(std::move(lines[i]));
        }

        return window(
            text(" Preview "),
            vbox(std::move(visibleLines)) | vscroll_indicator | frame | flex
        ) | flex;
    });
}
