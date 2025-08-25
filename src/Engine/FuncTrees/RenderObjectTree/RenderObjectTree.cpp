#include "RenderObjectTree.h"
#include "RenderObject.h"

Nebulite::RenderObjectTree::RenderObjectTree(RenderObject* domain, Nebulite::JSONTree* jsonTree)
    : FuncTree<Nebulite::ERROR_TYPE>("RenderObjectTree", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID, jsonTree), domain(domain) 
{
    // Initialize Expansions
    layout      = createExpansionOfType<RenderObjectTreeExpansion::Layout>();
    logging     = createExpansionOfType<RenderObjectTreeExpansion::Logging>();
    parenting   = createExpansionOfType<RenderObjectTreeExpansion::Parenting>();
    stateUpdate = createExpansionOfType<RenderObjectTreeExpansion::StateUpdate>();
}

//--------------------------------- 
// Necessary updates
void Nebulite::RenderObjectTree::update() {
    // Update all expansions
    layout->update();
    logging->update();
    parenting->update();
    stateUpdate->update();
}
