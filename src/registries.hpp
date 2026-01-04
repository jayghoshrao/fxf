#pragma once

#include <unordered_map>
#include <string>
#include <functional>
#include <map>
#include "command.hpp"
#include <ftxui/component/event.hpp>

class App;

class CommandRegistry {
public:
    using CommandFn = std::function<bool(const std::vector<std::string>&)>;

    explicit CommandRegistry(App& app) : m_app(app) {}

    CommandRegistry(const CommandRegistry&) = delete;
    CommandRegistry& operator=(const CommandRegistry&) = delete;
    CommandRegistry(CommandRegistry&&) = delete;
    CommandRegistry& operator=(CommandRegistry&&) = delete;

    void RegisterDefaultCommands();

    void Register(const std::string& name, CommandFn&& fn) {
        commands_.emplace(name, std::move(fn));
    }

    void Register(const std::string& name, Command&& command)
    {
        commands_.emplace(name, command);
    }

    bool Execute(const std::string& line) const;

    const Command& Get(std::string key) {
        if(auto it = commands_.find(key); it != commands_.end() )
        {
            return it->second;
        }

        return Command::Null;
    }

private:
    App& m_app;
    std::unordered_map<std::string, Command> commands_;
};

class KeybindRegistry {
public:
    explicit KeybindRegistry(App& app) : m_app(app) {}

    KeybindRegistry(const KeybindRegistry&) = delete;
    KeybindRegistry& operator=(const KeybindRegistry&) = delete;
    KeybindRegistry(KeybindRegistry&&) = delete;
    KeybindRegistry& operator=(KeybindRegistry&&) = delete;

    void RegisterDefaultKeybinds();

    void Register(ftxui::Event event, Command&& command) {
        map_.emplace(event, command);
    }

    bool Execute(ftxui::Event event) const;

private:
    App& m_app;
    std::map<ftxui::Event, Command> map_;
};


