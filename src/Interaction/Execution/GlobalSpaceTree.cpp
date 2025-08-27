#include "Interaction/Execution/GlobalSpaceTree.h"
#include "Core/GlobalSpace.h"       // Global Space for Nebulite

//-------------------------------------
// Linking ALL Functions to GlobalSpaceTree
Nebulite::Interaction::Execution::GlobalSpaceTree::GlobalSpaceTree(Nebulite::Core::GlobalSpace* domain, Nebulite::Interaction::Execution::JSONTree* jsonTree)
    : FuncTree<Nebulite::Constants::ERROR_TYPE>("Nebulite", Nebulite::Constants::ERROR_TYPE::NONE, Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID, jsonTree), domain(domain) 
{

  // Initialize Expansions
  general = createExpansionOfType<Nebulite::Expansion::GlobalSpace::General>();
  renderer = createExpansionOfType<Nebulite::Expansion::GlobalSpace::Renderer>();
  debug = createExpansionOfType<Nebulite::Expansion::GlobalSpace::Debug>();
  gui = createExpansionOfType<Nebulite::Expansion::GlobalSpace::GUI>();
  RenderObjectDraft = createExpansionOfType<Nebulite::Expansion::GlobalSpace::RenderObjectDraft>();

  // Initialize Variable Bindings here, due to circular dependency issues
  bindVariable(&domain->cmdVars.headless, "headless", "Set headless mode (no renderer)");
  bindVariable(&domain->cmdVars.recover,  "recover",  "Enable recoverable error mode");
}

//--------------------------------- 
// Necessary updates
void Nebulite::Interaction::Execution::GlobalSpaceTree::update() {
    // Update the JSON tree
    domain->global.getJSONTree()->update();

    // Update all expansions
    general->update();
    renderer->update();
    debug->update();
    gui->update();
    RenderObjectDraft->update();
}