#include "RenderObjectTree.h"
#include "RenderObject.h"

Nebulite::RenderObjectTree::RenderObjectTree(RenderObject* self)
    :   FuncTreeWrapper<Nebulite::ERROR_TYPE>("RenderObjectTree", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID),
        self(self) // local linkage for usage within the factory method
{
    // Initialize all expansions
    layout      = createExpansionOfType<RenderObjectTreeExpansion::Layout>();
    logging     = createExpansionOfType<RenderObjectTreeExpansion::Logging>();
    parenting   = createExpansionOfType<RenderObjectTreeExpansion::Parenting>();
    stateUpdate = createExpansionOfType<RenderObjectTreeExpansion::StateUpdate>();
}
