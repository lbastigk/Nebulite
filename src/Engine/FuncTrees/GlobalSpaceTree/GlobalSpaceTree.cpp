#include "GlobalSpaceTree.h"
#include "GlobalSpace.h"       // Global Space for Nebulite

//-------------------------------------
// Linking ALL Functions to GlobalSpaceTree
Nebulite::GlobalSpaceTree::GlobalSpaceTree(Nebulite::GlobalSpace* self)
    : FuncTree<Nebulite::ERROR_TYPE>("Nebulite", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID), self(self) 
{
    // Initialize Expansions
  general  = createExpansionOfType<GlobalSpaceTreeExpansion::General>();
  renderer = createExpansionOfType<GlobalSpaceTreeExpansion::Renderer>();
  debug    = createExpansionOfType<GlobalSpaceTreeExpansion::Debug>();
}