#include "DomainModule/RenderObject/RDM_Layout.hpp"
#include "Core/RenderObject.hpp"

//------------------------------------------
// Update
void Nebulite::DomainModule::RenderObject::Layout::update() {
    // Add FuncTree-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//------------------------------------------
// FuncTree-Bound Functions

Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::Layout::alignText(int argc, char* argv[]){
    return Nebulite::Constants::ErrorTable::FUNCTIONALL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::Layout::makeBox(int argc, char* argv[]){
    return Nebulite::Constants::ErrorTable::FUNCTIONALL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}