#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
#include <vector>
#include <iostream>
#include <sstream>

#include "app.hpp"
#include "utils.hpp"


class Command {
    using CommandFn = std::function<bool(const std::vector<std::string>&)>;

public:
    enum class ExecutionPolicy {
        Silent,
        Modal,
        Alias
    };


private:
    ExecutionPolicy m_execPolicy;
    CommandFn m_nativeCommandExecutor;
    std::string m_command;


public:
    static const Command Null;

    Command(CommandFn&& nativeCommandExec) :
        m_nativeCommandExecutor { std::move(nativeCommandExec)} {}

    Command(std::string command, ExecutionPolicy execPolicy) :
        m_command(command),
        m_execPolicy(execPolicy)
    {
    }

    bool Execute(std::string_view extraArgs = "") const;

    static Command::ExecutionPolicy StringToExecutionPolicy(std::string_view strPolicy);
};


