#include "Nebulite.hpp"
#include "Interaction/Execution/DomainModule.hpp"

namespace Nebulite::Interaction::Execution {

DomainModuleBase::DomainModuleBase(std::shared_ptr<FuncTree<Constants::Error>> funcTreePtr, Data::JsonScopeBase& w, Data::JsonScopeBase const& s)
    : funcTree(std::move(funcTreePtr)), moduleScope(w), settingsScope(s) {}

Data::JsonScopeBase& DomainModuleBase::getDoc() const {
    return moduleScope;
}

} // namespace Nebulite::Interaction::Execution
