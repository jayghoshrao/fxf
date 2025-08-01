#include "registries.hpp"
#include "appstate.hpp"

bool CommandRegistry::Execute(const std::string& line) const {
    AppState& appState = AppState::Instance();
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

        return it->second.Execute(args);
    }
    return false;
}

bool KeybindRegistry::Execute(std::string key) const{
    if(auto it = map_.find(key); it != map_.end())
    {
        it->second.Execute();
        return true;
    }
    return false;
}
