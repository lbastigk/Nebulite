#include "GlobalSpaceTree.h"
#include "GlobalSpace.h"       // Global Space for Nebulite

//-------------------------------------
// Linking ALL Functions to GlobalSpaceTree
Nebulite::GlobalSpaceTree::GlobalSpaceTree(Nebulite::GlobalSpace* self, Nebulite::JSONTree* jsonTree)
    : FuncTree<Nebulite::ERROR_TYPE>("Nebulite", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID, jsonTree), self(self) 
{
  // Initialize Expansions
  general  = createExpansionOfType<GlobalSpaceTreeExpansion::General>();
  renderer = createExpansionOfType<GlobalSpaceTreeExpansion::Renderer>();
  debug    = createExpansionOfType<GlobalSpaceTreeExpansion::Debug>();

  // Initialize Variable Bindings here, due to circular dependency issues
  bindVariable(&self->headless, "headless", "Set headless mode (no renderer)");
  bindVariable(&self->recover, "recover", "Enable recoverable error mode");
}