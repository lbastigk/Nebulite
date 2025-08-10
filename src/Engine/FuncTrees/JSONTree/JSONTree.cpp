#include "JSONTree.h"

Nebulite::JSONTree::JSONTree(Nebulite::JSON* self)
    : FuncTree<Nebulite::ERROR_TYPE>("JSONTree", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID), self(self) 
{
    // Initialize Expansions
    simpleData = createExpansionOfType<Nebulite::JSONTreeExpansion::SimpleData>();
    complexData = createExpansionOfType<Nebulite::JSONTreeExpansion::ComplexData>();
}