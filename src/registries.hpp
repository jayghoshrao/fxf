#pragma once

#include <unordered_map>
#include <string>
#include <functional>
#include <map>
#include "command.hpp"

class CommandRegistry {
    public:
        using CommandFn = std::function<bool(const std::vector<std::string>&)>;

        static CommandRegistry& Instance()
        {
            static CommandRegistry instance;
            return instance;
        }

        CommandRegistry() = default;
        CommandRegistry(const CommandRegistry&) = delete;
        CommandRegistry& operator=(const CommandRegistry&) = delete;
        CommandRegistry(CommandRegistry&&) = delete;
        CommandRegistry& operator=(CommandRegistry&&) = delete;

        void Register(const std::string& name, CommandFn&& fn) {
            commands_.emplace(name, std::move(fn));
        }

        void Register(const std::string& name, Command&& command)
        {
            commands_.emplace(name, command);
        }

        bool Execute(const std::string& line) const;


    private:
        std::unordered_map<std::string, Command> commands_;
};

class KeybindRegistry {
public:
    void Register(std::string event, Command&& command) {
        map_.emplace(event, command);
    }

    bool Execute(std::string key) const;

private:
    std::map<std::string, Command> map_;
};

