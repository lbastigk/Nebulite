#include "Nebulite.hpp"
#include "Interaction/Execution/DomainModuleBase.hpp"


namespace Nebulite::Interaction::Execution {

DomainModuleBase::DomainModuleBase(
    std::shared_ptr<FuncTree<Constants::Error, Domain&, Data::JsonScopeBase&>> funcTreePtr,
    Data::JsonScopeBase& w,
    Data::JsonScopeBase const& s
) : moduleScope(w), settingsScope(s), funcTree(std::move(funcTreePtr)) {}

DomainModuleBase::~DomainModuleBase() = default;

Constants::Error DomainModuleBase::update() { return Constants::ErrorTable::NONE(); }

void DomainModuleBase::reinit() {}

} // namespace Nebulite::Interaction::Execution
