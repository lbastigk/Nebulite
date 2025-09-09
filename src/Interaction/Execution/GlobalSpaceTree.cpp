#include "Interaction/Execution/GlobalSpaceTree.hpp"
#include "Core/GlobalSpace.hpp"       // Global Space for Nebulite

//------------------------------------------
// Linking ALL Functions to GlobalSpaceTree
Nebulite::Interaction::Execution::GlobalSpaceTree::GlobalSpaceTree(Nebulite::Core::GlobalSpace* domain, Nebulite::Interaction::Execution::JSONTree* jsonTree)
    : FuncTree<Nebulite::Constants::ERROR_TYPE>("Nebulite", Nebulite::Constants::ERROR_TYPE::NONE, Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID, jsonTree), domain(domain) 
{
  // Initialize DomainModules
  createDomainModuleOfType<Nebulite::DomainModule::GlobalSpace::General>();
  createDomainModuleOfType<Nebulite::DomainModule::GlobalSpace::Renderer>();
  createDomainModuleOfType<Nebulite::DomainModule::GlobalSpace::Debug>();
  createDomainModuleOfType<Nebulite::DomainModule::GlobalSpace::GUI>();
  createDomainModuleOfType<Nebulite::DomainModule::GlobalSpace::Input>();
  createDomainModuleOfType<Nebulite::DomainModule::GlobalSpace::RenderObjectDraft>();

  // Initialize Variable Bindings here, due to circular dependency issues
  bindVariable(&domain->cmdVars.headless, "headless", "Set headless mode (no renderer)");
  bindVariable(&domain->cmdVars.recover,  "recover",  "Enable recoverable error mode");
}

//------------------------------------------ 
// Necessary updates
void Nebulite::Interaction::Execution::GlobalSpaceTree::update() {
    // Update the JSON tree
    domain->global.getJSONTree()->update();

    // Update all DomainModules
    for(auto& module : modules){
        module->update();
    }
}