#include "Interaction/Execution/RenderObjectTree.h"
#include "Core/RenderObject.h"

Nebulite::Interaction::Execution::RenderObjectTree::RenderObjectTree(Nebulite::Core::RenderObject* domain, Nebulite::Interaction::Execution::JSONTree* jsonTree)
    : FuncTree<Nebulite::Constants::ERROR_TYPE>("RenderObjectTree", Nebulite::Constants::ERROR_TYPE::NONE, Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID, jsonTree), domain(domain) 
{
    // Initialize Expansions
    layout      = createExpansionOfType<Nebulite::Expansion::RenderObject::Layout>();
    logging     = createExpansionOfType<Nebulite::Expansion::RenderObject::Logging>();
    parenting   = createExpansionOfType<Nebulite::Expansion::RenderObject::Parenting>();
    stateUpdate = createExpansionOfType<Nebulite::Expansion::RenderObject::StateUpdate>();
}

//--------------------------------- 
// Necessary updates
void Nebulite::Interaction::Execution::RenderObjectTree::update() {
    // Update all expansions
    layout->update();
    logging->update();
    parenting->update();
    stateUpdate->update();
}
