#include "MainTree.h"
#include "GlobalSpace.h"       // Global Space for Nebulite
#include "Invoke.h"            // Invoke for parsing expressions


//-------------------------------------
// Linking ALL Functions to MainTree
Nebulite::MainTree::MainTree(Nebulite::Invoke* invokeLinkage, Nebulite::GlobalSpace* globalSpaceLinkage)
    : FuncTreeWrapper("Nebulite", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID),  // Passing necessary error types to FuncTreeWrapper
    invokeLinkage(invokeLinkage),           // local linkage for usage within the factory method
    globalSpaceLinkage(globalSpaceLinkage)
{
  // Initialize all expansions
  general  = createExpansionOfType<MainTreeExpansion::General>();
  renderer = createExpansionOfType<MainTreeExpansion::Renderer>();
  debug    = createExpansionOfType<MainTreeExpansion::Debug>();
}