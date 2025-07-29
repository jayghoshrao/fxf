#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <iostream>
#include <sstream>

class CommandRegistry {
public:
    using CommandFn = std::function<bool(const std::vector<std::string>&)>;

    void Register(const std::string& name, CommandFn fn) {
        commands_[name] = std::move(fn);
    }

    bool Execute(const std::string& line) const {
        std::istringstream iss(line);
        std::string cmd;
        std::vector<std::string> args;

        while(iss >> cmd)
            args.push_back(cmd);

        if(args.empty())
            return false;

        auto it = commands_.find(args[0]);
        if(it != commands_.end()) {
            return it->second(args);
        } else {
            std::cerr << "Unknown command: " << args[0] << "\n";
            return false;
        }
    }

private:
    std::unordered_map<std::string, CommandFn> commands_;
};
