#include "MainTree.h"
#include "GlobalSpace.h"       // Global Space for Nebulite
#include "Invoke.h"            // Invoke for parsing expressions


//-------------------------------------
// Linking ALL Functions to MainTree
Nebulite::MainTree::MainTree(Nebulite::Invoke* invokeLinkage, Nebulite::GlobalSpace* globalSpaceLinkage)
    : FuncTreeWrapper("Nebulite", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID), // Passing necessary error types to FuncTreeWrapper
      // Passing linkages to MainTree Categories derived from MT__Wrapper:
      general   (invokeLinkage, globalSpaceLinkage, &this->funcTree), 
      renderer  (invokeLinkage, globalSpaceLinkage, &this->funcTree),
      debug     (invokeLinkage, globalSpaceLinkage, &this->funcTree) 
{
  general.initialize();  // Initialize General Category
  renderer.initialize(); // Initialize Renderer Category
  debug.initialize();    // Initialize Debug Category
}