# fxf

A vim/fzf/lazygit inspired text picker and browser written in C++23.

fxf provides an interactive terminal UI for selecting and browsing delimited text files with fuzzy search capabilities.

## Features

- Vim-style keybindings (`j`/`k`, `gg`/`G`, `Ctrl+D`/`Ctrl+U`)
- Fuzzy search with `/`
- Command mode with `:`
- Customizable view templates for displaying columns
- Extensible command and keybind system
- URL detection and opening

## Build

```bash
cmake -S . -B build && cmake --build build
```

Or use the Makefile wrapper:
```bash
make build
```

### Dependencies (fetched automatically via CMake)

- [FTXUI](https://github.com/arthursonzogni/ftxui) - Terminal UI framework
- [CLI11](https://github.com/CLIUtils/CLI11) - Command-line argument parsing
- [rapidfuzz-cpp](https://github.com/rapidfuzz/rapidfuzz-cpp) - Fuzzy matching
- [Catch2](https://github.com/catchorg/Catch2) - Testing framework

## Usage

```bash
fxf <file> [-d <delimiter>]
```

### Examples

```bash
# Browse a pipe-delimited file (default delimiter)
fxf data.txt

# Browse a CSV file
fxf data.csv -d ,

# List installed AUR packages
fxf <(paru -Qmq)

# Browse PATH directories
echo $PATH | tr ':' '\n' | fxf /dev/stdin
```

## Keybindings

| Key | Action |
|-----|--------|
| `j` / `Down` | Move down |
| `k` / `Up` | Move up |
| `gg` | Jump to top |
| `G` | Jump to bottom |
| `Ctrl+F` / `PageDown` | Page down |
| `Ctrl+B` / `PageUp` | Page up |
| `Ctrl+D` | Half page down |
| `Ctrl+U` | Half page up |
| `/` | Enter search mode |
| `:` | Enter command mode |
| `Enter` | Select current entry and exit |
| `q` | Quit |
| `o` | Open first URL in current row |
| `0-9` | Show column N |
| `=` | Show all columns |

### Text Input (Search/Command Mode)

| Key | Action |
|-----|--------|
| `Ctrl+A` | Move cursor to beginning |
| `Ctrl+E` | Move cursor to end |
| `Ctrl+U` | Delete from beginning to cursor |
| `Ctrl+K` | Delete from cursor to end |
| `Ctrl+W` | Delete word before cursor |
| `Alt+B` | Move back one word |
| `Alt+F` | Move forward one word |
| `Escape` | Cancel and exit input mode |

## Commands

Commands are entered in command mode (`:`)

| Command | Description |
|---------|-------------|
| `quit` | Exit the application |
| `read <delimiter> <file>` | Load a new file |
| `view <template>` | Set view template (e.g., `view {0} - {1}`) |
| `show [N]` | Show column N or all columns |
| `delete` | Delete current row |
| `open` | Open first URL in current row |
| `select` | Output current entry (with view template) and exit |
| `bind <key> <type> <cmd>` | Bind a key to a command |
| `command <name> <type> <cmd>` | Create a custom command |

### Command Types

- `alias` - Internal command alias
- `silent` - Run shell command silently
- `modal` - Run command and display output

### Template System

Templates control how rows are displayed:
- `{}` - All columns (pipe-separated)
- `{0}`, `{1}`, etc. - Specific column by index

Example: `view {0} | {2}` shows only the first and third columns.

## Testing

```bash
# Build and run tests
cmake --build build --target tests
./build/tests

# Or use ctest
ctest --test-dir build
```

## License

See LICENSE file for details.
