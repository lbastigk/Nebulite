#include "DomainModule/RenderObject/RDM_Layout.h"
#include "Core/RenderObject.h"

//-------------------------------
// Update
void Nebulite::DomainModule::RenderObject::Layout::update() {
    // Add FuncTree-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//-------------------------------
// FuncTree-Bound Functions

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::RenderObject::Layout::alignText(int argc, char* argv[]){
    return Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::RenderObject::Layout::makeBox(int argc, char* argv[]){
    return Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}