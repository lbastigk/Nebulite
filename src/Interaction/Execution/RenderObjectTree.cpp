#include "Interaction/Execution/RenderObjectTree.h"
#include "Core/RenderObject.h"

Nebulite::Interaction::Execution::RenderObjectTree::RenderObjectTree(Nebulite::Core::RenderObject* domain, Nebulite::Interaction::Execution::JSONTree* jsonTree)
    : FuncTree<Nebulite::Constants::ERROR_TYPE>("RenderObjectTree", Nebulite::Constants::ERROR_TYPE::NONE, Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID, jsonTree), domain(domain) 
{
    // Initialize DomainModules
    layout      = createDomainModuleOfType<Nebulite::DomainModule::RenderObject::Layout>();
    logging     = createDomainModuleOfType<Nebulite::DomainModule::RenderObject::Logging>();
    parenting   = createDomainModuleOfType<Nebulite::DomainModule::RenderObject::Parenting>();
    stateUpdate = createDomainModuleOfType<Nebulite::DomainModule::RenderObject::StateUpdate>();
}

//------------------------------------------ 
// Necessary updates
void Nebulite::Interaction::Execution::RenderObjectTree::update() {
    // Update all DomainModules
    layout->update();
    logging->update();
    parenting->update();
    stateUpdate->update();
}
