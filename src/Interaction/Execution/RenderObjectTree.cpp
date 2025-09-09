#include "Interaction/Execution/RenderObjectTree.hpp"
#include "Core/RenderObject.hpp"

Nebulite::Interaction::Execution::RenderObjectTree::RenderObjectTree(Nebulite::Core::RenderObject* domain, Nebulite::Interaction::Execution::JSONTree* jsonTree)
    : FuncTree<Nebulite::Constants::ERROR_TYPE>("RenderObjectTree", Nebulite::Constants::ERROR_TYPE::NONE, Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID, jsonTree), domain(domain) 
{
    // Initialize DomainModules
    createDomainModuleOfType<Nebulite::DomainModule::RenderObject::Layout>();
    createDomainModuleOfType<Nebulite::DomainModule::RenderObject::Logging>();
    createDomainModuleOfType<Nebulite::DomainModule::RenderObject::Parenting>();
    createDomainModuleOfType<Nebulite::DomainModule::RenderObject::StateUpdate>();
}

//------------------------------------------ 
// Necessary updates
void Nebulite::Interaction::Execution::RenderObjectTree::update() {
    // Update all DomainModules
    for(auto& module : modules){
        module->update();
    }
}
