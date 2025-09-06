#include "command.hpp"
#include "registries.hpp"
#include "app.hpp"

/* static */
const Command Command::Null([](std::vector<std::string>){return false;});

bool Command::Execute(std::string extraArgs /*= ""*/) const {
    std::string command = m_command + ' '  + extraArgs;

    auto& app = App::Instance();
    auto commandstr = app.controls.lines.Substitute(command, app.controls.selector);

    if(m_nativeCommandExecutor)
    {
        std::istringstream iss(command);
        std::string cmd;
        std::vector<std::string> args;

        while(iss >> cmd)
        {
            args.push_back(cmd);
        }
        return m_nativeCommandExecutor(args);
    }

    // native commands do not have/need an execPolicy.
    switch(m_execPolicy)
    {
        case ExecutionPolicy::Alias:
            {
                return CommandRegistry::Instance().Execute(commandstr);
            }
        case ExecutionPolicy::Silent:
            {
                int err = std::system(commandstr.c_str());
                return err == 0 ? true : false;
            }
        case ExecutionPolicy::Modal:
            {
                app.controls.display.string = ExecAndCapture(commandstr.c_str());
                app.controls.display.isActive = true;
                return true;
            }
        default:
            break;
    }
    return false;
}

/* static */
Command::ExecutionPolicy Command::StringToExecutionPolicy(std::string strPolicy)
{
    auto strPolicyTrimmed = trim(strPolicy);

    if(strPolicyTrimmed == "alias") return Command::ExecutionPolicy::Alias;
    if(strPolicyTrimmed == "silent") return Command::ExecutionPolicy::Silent;
    if(strPolicyTrimmed == "modal") return Command::ExecutionPolicy::Modal;

    return Command::ExecutionPolicy::Alias;
}
