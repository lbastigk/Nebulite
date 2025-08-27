#include "Interaction/Execution/JSONTree.h"

Nebulite::Interaction::Execution::JSONTree::JSONTree(Nebulite::Utility::JSON* domain)
    : FuncTree<Nebulite::Constants::ERROR_TYPE>("JSONTree", Nebulite::Constants::ERROR_TYPE::NONE, Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID), domain(domain) 
{
    // Initialize Expansions
    simpleData = createExpansionOfType<Nebulite::Expansion::JSON::SimpleData>();
    complexData = createExpansionOfType<Nebulite::Expansion::JSON::ComplexData>();
}

//--------------------------------- 
// Necessary updates
void Nebulite::Interaction::Execution::JSONTree::update() {
    // Update all expansions
    simpleData->update();
    complexData->update();
}
