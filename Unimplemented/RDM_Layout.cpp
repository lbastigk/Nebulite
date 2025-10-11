#include "DomainModule/Unimplemented/RDM_Layout.hpp"
#include "Core/RenderObject.hpp"

namespace Nebulite::DomainModule::RenderObject{

//------------------------------------------
// Update
Nebulite::Constants::Error Layout::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

Nebulite::Constants::Error Layout::alignText(int argc, char* argv[]){
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
Nebulite::Constants::Error Layout::makeBox(int argc, char* argv[]){
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}

}   // namespace Nebulite::DomainModule::RenderObject