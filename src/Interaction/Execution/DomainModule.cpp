#include "Nebulite.hpp"
#include "Interaction/Execution/DomainModule.hpp"

namespace Nebulite::Interaction::Execution {

DomainModuleBase::DomainModuleBase(std::shared_ptr<FuncTree<Constants::Error>> funcTreePtr, Data::JsonScopeBase& w)
    : funcTree(std::move(funcTreePtr)), moduleScope(w) {}

Data::JsonScopeBase& DomainModuleBase::getDoc() const {
    return moduleScope;
}

} // namespace Nebulite::Interaction::Execution
