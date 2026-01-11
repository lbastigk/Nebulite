#include "Nebulite.hpp"
#include "Interaction/Execution/DomainModule.hpp"

namespace Nebulite::Interaction::Execution {

DomainModuleBase::DomainModuleBase(std::shared_ptr<FuncTree<Constants::Error>> funcTreePtr, Data::JsonScopeBase& w, Data::JsonScopeBase const& s)
    : moduleScope(w), settingsScope(s), funcTree(std::move(funcTreePtr)) {}

} // namespace Nebulite::Interaction::Execution
