#include "MainTree.h"
#include "GlobalSpace.h"       // Global Space for Nebulite
#include "Invoke.h"            // Invoke for parsing expressions


//-------------------------------------
// Linking ALL Functions to MainTree
Nebulite::MainTree::MainTree(Nebulite::Invoke* invokeLinkage, Nebulite::GlobalSpace* globalSpaceLinkage)
    : FuncTreeWrapper("Nebulite", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID) // Passing necessary error types to FuncTreeWrapper
{
  // Initialize the categories
  general   = std::make_unique<Nebulite::MainTreeExpansion::General>(invokeLinkage, globalSpaceLinkage, &this->funcTree);
  renderer  = std::make_unique<Nebulite::MainTreeExpansion::Renderer>(invokeLinkage, globalSpaceLinkage, &this->funcTree);
  debug     = std::make_unique<Nebulite::MainTreeExpansion::Debug>(invokeLinkage, globalSpaceLinkage, &this->funcTree);
}