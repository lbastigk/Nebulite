#include "Interaction/Execution/JSONTree.hpp"

Nebulite::Interaction::Execution::JSONTree::JSONTree(Nebulite::Utility::JSON* domain)
    : FuncTree<Nebulite::Constants::ERROR_TYPE>("JSONTree", Nebulite::Constants::ERROR_TYPE::NONE, Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID), domain(domain) 
{
    // Initialize DomainModules
    simpleData = createDomainModuleOfType<Nebulite::DomainModule::JSON::SimpleData>();
    complexData = createDomainModuleOfType<Nebulite::DomainModule::JSON::ComplexData>();
}

//------------------------------------------ 
// Necessary updates
void Nebulite::Interaction::Execution::JSONTree::update() {
    // Update all DomainModules
    simpleData->update();
    complexData->update();
}
