#include "utils.hpp"

#include <ranges>
#include <memory>
#include <regex>
#include <unistd.h>
#include <sys/wait.h>

using namespace ftxui;

std::vector<std::string> split_csv_line(std::string_view line, char delimiter /*= ','*/)
{
    return line
        | std::views::split(delimiter)
        | std::views::transform([](auto&& subrange) {
              return std::string(&*subrange.begin(), std::ranges::distance(subrange));
          })
        | std::ranges::to<std::vector<std::string>>();
}

std::vector<std::string_view> split_csv_line_view(std::string_view line, char delimiter /*= ','*/)
{
    std::vector<std::string_view> result;
    for (auto&& subrange : std::views::split(line, delimiter))
    {
        result.emplace_back(&*subrange.begin(), std::ranges::distance(subrange));
    }

    return result;
}

std::string EventToString(const Event& event) {
    if (event == Event::Return) return "return";
    if (event == Event::Escape) return "escape";
    if (event == Event::Tab) return "tab";
    if (event == Event::Backspace) return "backspace";
    if (event == Event::Delete) return "delete";
    if (event == Event::ArrowUp) return "arrowup";
    if (event == Event::ArrowDown) return "arrowdown";
    if (event == Event::ArrowLeft) return "arrowleft";
    if (event == Event::ArrowRight) return "arrowright";
    if (event == Event::Home) return "home";
    if (event == Event::End) return "end";
    if (event == Event::PageUp) return "pageup";
    if (event == Event::PageDown) return "pagedown";

    // Handle single characters
    if (event.is_character()) {
        return event.character();
    }

    if (event == Event::F1) return "f1";
    if (event == Event::F2) return "f2";
    if (event == Event::F3) return "f3";
    if (event == Event::F4) return "f4";
    if (event == Event::F5) return "f5";
    if (event == Event::F6) return "f6";
    if (event == Event::F7) return "f7";
    if (event == Event::F8) return "f8";
    if (event == Event::F9) return "f9";
    if (event == Event::F10) return "f10";
    if (event == Event::F11) return "f11";
    if (event == Event::F12) return "f12";

    // Return a default string for unknown events
    return "unknown";
}

std::string ExecAndCapture(const std::string& cmd) {
    char buffer[128];
    std::string result = "";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer, sizeof buffer, pipe.get()) != nullptr) {
        result += buffer;
    }

    return result;
}

std::string substitute_template(std::string_view template_str, const std::vector<std::string>& data) {
    std::string result;
    result.reserve(template_str.size() * 2);

    std::string joined_data;
    bool joined_computed = false;

    size_t i = 0;
    while (i < template_str.size()) {
        if (template_str[i] == '{') {
            size_t close = template_str.find('}', i + 1);
            if (close != std::string_view::npos) {
                std::string_view inner = template_str.substr(i + 1, close - i - 1);

                if (inner.empty()) {
                    // {} placeholder - lazy compute joined_data
                    if (!joined_computed) {
                        if (!data.empty()) {
                            size_t total_len = data[0].size();
                            for (size_t j = 1; j < data.size(); ++j) {
                                total_len += 3 + data[j].size();
                            }
                            joined_data.reserve(total_len);
                            joined_data = data[0];
                            for (size_t j = 1; j < data.size(); ++j) {
                                joined_data += " | ";
                                joined_data += data[j];
                            }
                        }
                        joined_computed = true;
                    }
                    result += joined_data;
                } else {
                    // Parse {N} placeholder
                    size_t idx = 0;
                    bool valid = true;
                    for (char c : inner) {
                        if (c >= '0' && c <= '9') {
                            idx = idx * 10 + (c - '0');
                        } else {
                            valid = false;
                            break;
                        }
                    }

                    if (valid && idx < data.size()) {
                        result += data[idx];
                    } else {
                        result.append(template_str.substr(i, close - i + 1));
                    }
                }
                i = close + 1;
                continue;
            }
        }
        result += template_str[i];
        ++i;
    }

    return result;
}

std::string trim(std::string_view str) {
    const auto first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string_view::npos)
        return ""; // all whitespace
    const auto last = str.find_last_not_of(" \t\n\r\f\v");
    return std::string{str.substr(first, (last - first + 1))};
}

std::vector<std::string> ExtractURLs(const std::string& text) {
    static const std::regex url_regex(
        R"((https?://(?:www\.)?[-a-zA-Z0-9@:%._+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b(?:[-a-zA-Z0-9()@:%_+.~#?&/=]*)))",
        std::regex::icase
    );

    std::vector<std::string> urls;
    auto begin = std::sregex_iterator(text.begin(), text.end(), url_regex);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        urls.push_back(it->str());
    }
    return urls;
}

std::string ExtractFirstURL(const std::string& text) {
    static const std::regex url_regex(
        R"((https?://(?:www\.)?[-a-zA-Z0-9@:%._+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b(?:[-a-zA-Z0-9()@:%_+.~#?&/=]*)))",
        std::regex::icase
    );

    std::smatch match;
    if (std::regex_search(text, match, url_regex)) {
        return match.str(0);
    }
    return {};
}

std::vector<std::string> SplitCommand(std::string_view cmd) {
    std::vector<std::string> args;
    std::string current;
    bool in_single_quote = false;
    bool in_double_quote = false;
    bool escaped = false;

    for (char c : cmd) {
        if (escaped) {
            current += c;
            escaped = false;
            continue;
        }

        if (c == '\\' && !in_single_quote) {
            escaped = true;
            continue;
        }

        if (c == '\'' && !in_double_quote) {
            in_single_quote = !in_single_quote;
            continue;
        }

        if (c == '"' && !in_single_quote) {
            in_double_quote = !in_double_quote;
            continue;
        }

        if (std::isspace(c) && !in_single_quote && !in_double_quote) {
            if (!current.empty()) {
                args.push_back(std::move(current));
                current.clear();
            }
            continue;
        }

        current += c;
    }

    if (!current.empty()) {
        args.push_back(std::move(current));
    }

    return args;
}

int ExecNoShell(std::string_view cmd) {
    auto args = SplitCommand(cmd);
    if (args.empty()) return -1;

    std::vector<char*> argv;
    for (auto& arg : args) {
        argv.push_back(arg.data());
    }
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid == -1) {
        return -1;
    }

    if (pid == 0) {
        execvp(argv[0], argv.data());
        _exit(127);
    }

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}
