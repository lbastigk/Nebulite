//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <utility>

// Nebulite
#include "Constants/Event.hpp"
#include "Interaction/Execution/DomainModuleBase.hpp"
#include "Interaction/Execution/DomainTree.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Execution {

DomainModuleBase::DomainModuleBase(
    std::shared_ptr<DomainTree> funcTreePtr,
    Data::JsonScope& w,
    Data::JsonScope const& s
) : moduleScope(w), settingsScope(s), funcTree(std::move(funcTreePtr)) {}

DomainModuleBase::~DomainModuleBase() = default;

Constants::Event DomainModuleBase::updateHook() { return Constants::Event::Success; }

void DomainModuleBase::reinit() {}

} // namespace Nebulite::Interaction::Execution
