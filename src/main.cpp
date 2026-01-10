#include <iostream>
#include <unistd.h>
#include <CLI/CLI.hpp>

#include "app.hpp"

using namespace ftxui;

int main(int argc, char* argv[]) {

    App& app = App::Instance();
    CLI::App args{"fxf - interactive text picker"};

    std::string filename;
    char delimiter = '|';
    args.add_option("file", filename, "File to read (optional if piping data)");
    args.add_option("-d,--delimiter", delimiter, "Delimiter");

    CLI11_PARSE(args, argc, argv);

    bool stdin_is_pipe = !isatty(STDIN_FILENO);
    bool stdout_is_pipe = !isatty(STDOUT_FILENO);

    // If stdin is a pipe, read data from it
    if (stdin_is_pipe) {
        std::string line;
        while (std::getline(std::cin, line)) {
            app.state.lines.AddLine(line, delimiter);
        }

        // Reopen stdin from /dev/tty for keyboard input
        if (!freopen("/dev/tty", "r", stdin)) {
            std::cerr << "Error: Cannot open /dev/tty for terminal input\n";
            return EXIT_FAILURE;
        }

        app.state.delimiter = delimiter;
    }

    // If stdout is a pipe, save it and redirect stdout to /dev/tty for the TUI
    int saved_stdout = -1;
    if (stdout_is_pipe) {
        saved_stdout = dup(STDOUT_FILENO);
        if (saved_stdout == -1) {
            std::cerr << "Error: Cannot save stdout\n";
            return EXIT_FAILURE;
        }
        if (!freopen("/dev/tty", "w", stdout)) {
            std::cerr << "Error: Cannot open /dev/tty for terminal output\n";
            return EXIT_FAILURE;
        }
    }

    app.CreateGUI();

    // Load from file if provided (overrides piped data)
    if (!filename.empty()) {
        app.Load(filename, delimiter);
    } else if (stdin_is_pipe) {
        // Data already loaded from pipe, initialize filter
        app.controls.viewTemplate = "{}";
        app.ResetFilter();
        app.controls.selected = 0;
    } else {
        // No input provided
        std::cerr << "Error: No input provided. Provide a file or pipe data.\n";
        return EXIT_FAILURE;
    }

    app.Loop();

    // Restore stdout if it was redirected
    if (saved_stdout != -1) {
        fflush(stdout);
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }

    if (!app.state.output.empty()) {
        std::cout << app.state.output << std::endl;
    }

    return EXIT_SUCCESS;
}
