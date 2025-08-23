#include "GlobalSpaceTree.h"
#include "GlobalSpace.h"       // Global Space for Nebulite

//-------------------------------------
// Linking ALL Functions to GlobalSpaceTree
Nebulite::GlobalSpaceTree::GlobalSpaceTree(Nebulite::GlobalSpace* domain, Nebulite::JSONTree* jsonTree)
    : FuncTree<Nebulite::ERROR_TYPE>("Nebulite", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID, jsonTree), domain(domain) 
{

  // Initialize Expansions
  general = createExpansionOfType<GlobalSpaceTreeExpansion::General>();
  renderer = createExpansionOfType<GlobalSpaceTreeExpansion::Renderer>();
  debug = createExpansionOfType<GlobalSpaceTreeExpansion::Debug>();
  gui = createExpansionOfType<GlobalSpaceTreeExpansion::GUI>();
  RenderObjectDraft = createExpansionOfType<GlobalSpaceTreeExpansion::RenderObjectDraft>();

  // Initialize Variable Bindings here, due to circular dependency issues
  bindVariable(&domain->cmdVars.headless, "headless", "Set headless mode (no renderer)");
  bindVariable(&domain->cmdVars.recover,  "recover",  "Enable recoverable error mode");
}

//--------------------------------- 
// Necessary updates
void Nebulite::GlobalSpaceTree::update() {
    // Update the JSON tree
    domain->global.getJSONTree()->update();

    // Update all expansions
    general->update();
    renderer->update();
    debug->update();
    gui->update();
    RenderObjectDraft->update();
}