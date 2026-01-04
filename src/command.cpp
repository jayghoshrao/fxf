#include "command.hpp"
#include "app.hpp"
#include "utils.hpp"

/* static */
const Command Command::Null([](std::vector<std::string>){return false;});

bool Command::Execute(std::string_view extraArgs /*= ""*/) const {
    std::string command = m_command + ' '  + std::string{extraArgs};

    auto& app = App::Instance();
    auto commandstr = app.state.lines.Substitute(command, app.controls.selected);

    if(m_nativeCommandExecutor)
    {
        auto args = SplitCommand(command);
        return m_nativeCommandExecutor(args);
    }

    // native commands do not have/need an execPolicy.
    switch(m_execPolicy)
    {
        case ExecutionPolicy::Alias:
            {
                return app.commands.Execute(commandstr);
            }
        case ExecutionPolicy::Silent:
            {
                int err = ExecNoShell(commandstr);
                return err == 0;
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
Command::ExecutionPolicy Command::StringToExecutionPolicy(std::string_view strPolicy)
{
    auto strPolicyTrimmed = trim(strPolicy);

    if(strPolicyTrimmed == "alias") return Command::ExecutionPolicy::Alias;
    if(strPolicyTrimmed == "silent") return Command::ExecutionPolicy::Silent;
    if(strPolicyTrimmed == "modal") return Command::ExecutionPolicy::Modal;

    return Command::ExecutionPolicy::Alias;
}
