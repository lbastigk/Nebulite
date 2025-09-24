#include "DomainModule/RenderObject/RDM_StateUpdate.hpp"
#include "Core/RenderObject.hpp"

//------------------------------------------
// Update
void Nebulite::DomainModule::RenderObject::StateUpdate::update() {
    // Add FuncTree-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//------------------------------------------
// FuncTree-Bound Functions

Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::StateUpdate::deleteObject(int argc, char* argv[]){
    domain->flag.deleteFromScene = true;
    return Nebulite::Constants::ErrorTable::NONE();
}
Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::StateUpdate::updateText(int argc, char* argv[]){
    domain->flag.calculateText = true;
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::StateUpdate::reloadInvokes(int argc, char* argv[]){
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}

Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::StateUpdate::addInvoke(int argc, char* argv[]){
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::StateUpdate::removeInvoke(int argc, char* argv[]){
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::StateUpdate::removeAllInvokes(int argc, char* argv[]){
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}