#include "JSONTree.h"

Nebulite::JSONTree::JSONTree(Nebulite::JSON* domain)
    : FuncTree<Nebulite::ERROR_TYPE>("JSONTree", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID), domain(domain) 
{
    // Initialize Expansions
    simpleData = createExpansionOfType<Nebulite::JSONTreeExpansion::SimpleData>();
    complexData = createExpansionOfType<Nebulite::JSONTreeExpansion::ComplexData>();
}

//--------------------------------- 
// Necessary updates
void Nebulite::JSONTree::update() {
    // Update all expansions
    simpleData->update();
    complexData->update();
}
