#include "RenderObjectTree.h"
#include "RenderObject.h"

Nebulite::RenderObjectTree::RenderObjectTree(RenderObject* self)
    : FuncTreeWrapper<Nebulite::ERROR_TYPE>("RenderObjectTree", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID)
{
    // Initialize the RenderObjectTree with the self object and function tree pointer
    data = std::make_unique<Nebulite::RenderObjectTreeExpansion::Data>(self, &funcTree);
    layout = std::make_unique<Nebulite::RenderObjectTreeExpansion::Layout>(self, &funcTree);
    logging = std::make_unique<Nebulite::RenderObjectTreeExpansion::Logging>(self, &funcTree);
    parenting = std::make_unique<Nebulite::RenderObjectTreeExpansion::Parenting>(self, &funcTree);
    stateUpdate = std::make_unique<Nebulite::RenderObjectTreeExpansion::StateUpdate>(self, &funcTree);
}
