#include "DomainModule/Unimplemented/GDM_StateManagement.hpp"

void Nebulite::DomainModule::GlobalSpace::StateManagement::update(){
    // Nothing to do for now
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::StateManagement::stateLoad(int argc, char* argv[]){ 
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::StateManagement::stateSave(int argc, char* argv[]){
    // <stateName>
    // Change std::string Nebulite::stateName to name
    // Check if dir ./States/stateName exists

    // If any env is deloaded, save in stateName

    // Every load of any file must be linked to state! If file exists in state load from there
    // if not, load from usual path

    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}