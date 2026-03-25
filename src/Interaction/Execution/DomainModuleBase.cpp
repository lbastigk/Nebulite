#include "Nebulite.hpp"
#include "Interaction/Execution/DomainModuleBase.hpp"


namespace Nebulite::Interaction::Execution {

DomainModuleBase::DomainModuleBase(
    std::shared_ptr<FuncTree<Constants::Event, Domain&, Data::JsonScope&>> funcTreePtr,
    Data::JsonScope& w,
    Data::JsonScope const& s
) : moduleScope(w), settingsScope(s), funcTree(std::move(funcTreePtr)) {}

DomainModuleBase::~DomainModuleBase() = default;

Constants::Event DomainModuleBase::updateHook() { return Constants::Event::Success; }

void DomainModuleBase::reinit() {}

} // namespace Nebulite::Interaction::Execution
