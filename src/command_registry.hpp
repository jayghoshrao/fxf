#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <iostream>
#include <sstream>

#include "appstate.hpp"
#include "utils.hpp"


class Command {
    using CommandFn = std::function<bool(const std::vector<std::string>&)>;

public:
    enum class ExecutionPolicy {
        Native=0,
        Silent,
        Modal
    };


private:
    ExecutionPolicy m_execPolicy;
    CommandFn m_nativeCommandExecutor;
    std::string m_command;


public:

    Command(CommandFn&& nativeCommandExec) :
        m_execPolicy{ExecutionPolicy::Native},
        m_nativeCommandExecutor { std::move(nativeCommandExec)} {}

    Command(std::string command, ExecutionPolicy execPolicy = ExecutionPolicy::Silent) :
        m_command(command), 
        m_execPolicy(execPolicy) {}

    bool Execute(std::string extraArgs = "") const {

        std::string command = m_command + ' '  + extraArgs;

        auto& appState = GetAppState();
        auto selected_line_split = split_csv_line(appState.lines[appState.selector]);
        auto commandstr = substitute_template(command, selected_line_split);

        switch(m_execPolicy)
        {
            case ExecutionPolicy::Native:
                {
                    std::istringstream iss(command);
                    std::string cmd;
                    std::vector<std::string> args;

                    while(iss >> cmd)
                    {
                        args.push_back(cmd);
                    }

                    if(m_nativeCommandExecutor)
                    {
                        return m_nativeCommandExecutor(args);
                    }
                }
                break;
            case ExecutionPolicy::Silent:
                {
                    int err = std::system(command.c_str());
                    return err == 0 ? true : false;
                    break;
                }
            case ExecutionPolicy::Modal:
                appState.display.string = ExecAndCapture(command.c_str());
                appState.display.isShown = true;
                return true;
                break;
            default:
                break;
        }
        return false;
    }
};

class CommandRegistry {
    public:
        using CommandFn = std::function<bool(const std::vector<std::string>&)>;

        void Register(const std::string& name, CommandFn&& fn) {
            commands_.emplace(name, std::move(fn));
        }

        void Register(const std::string& name, std::string command, Command::ExecutionPolicy execPolicy)
        {
            commands_.emplace(name, Command{command, execPolicy});
        }

        bool Execute(const std::string& line) const {
            auto& appState = GetAppState();
            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;
            if(auto it = commands_.find(cmd); it != commands_.end())
            {
                auto args = line 
                    | std::views::split(' ') 
                    | std::views::drop(1) 
                    | std::views::join_with(' ') 
                    | std::ranges::to<std::string>();

                it->second.Execute(args);
            }
            return false;
        }

    private:
        std::unordered_map<std::string, Command> commands_;
};
