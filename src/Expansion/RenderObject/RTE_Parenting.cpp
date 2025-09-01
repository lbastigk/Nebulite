#include "DomainModule/RenderObject/RDM_Parenting.h"
#include "Core/RenderObject.h"

//------------------------------------------
// Update
void Nebulite::DomainModule::RenderObject::Parenting::update() {
    // Add FuncTree-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//------------------------------------------
// FuncTree-Bound Functions

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::RenderObject::Parenting::addChildren(int argc, char* argv[]){
    // Idea: <link> <name>
    // Store in a map
    // E.g. add-children Pants/Jeans.jsonc myPants
    return Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::RenderObject::Parenting::removeChildren(int argc, char* argv[]){
    // Idea: <link> <name>
    // Store in a map
    // E.g. remove-children myPants
    return Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::RenderObject::Parenting::removeAllChildren(int argc, char* argv[]){
    return Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

