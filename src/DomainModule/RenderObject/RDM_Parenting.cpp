#include "DomainModule/RenderObject/RDM_Parenting.hpp"
#include "Core/RenderObject.hpp"

//------------------------------------------
// Update
void Nebulite::DomainModule::RenderObject::Parenting::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//------------------------------------------
// Domain-Bound Functions

Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::Parenting::addChildren(int argc, char* argv[]){
    // Idea: <link> <name>
    // Store in a map
    // E.g. add-children Pants/Jeans.jsonc myPants
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::Parenting::removeChildren(int argc, char* argv[]){
    // Idea: <link> <name>
    // Store in a map
    // E.g. remove-children myPants
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::Parenting::removeAllChildren(int argc, char* argv[]){
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}

