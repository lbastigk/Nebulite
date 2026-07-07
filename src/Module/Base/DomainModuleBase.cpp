//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <utility>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Interaction/Execution/DomainTree.hpp"
#include "Nebulite/Module/Base/DomainModuleBase.hpp"

//------------------------------------------
namespace Nebulite::Module::Base {

DomainModuleBase::DomainModuleBase(
    std::shared_ptr<Interaction::Execution::DomainTree> funcTreePtr,
    Data::JsonScope& w,
    Data::JsonScope const& s
) : moduleScope(w), settingsScope(s), funcTree(std::move(funcTreePtr)) {}

DomainModuleBase::~DomainModuleBase() = default;

Constants::Event DomainModuleBase::updateHook() { return Constants::Event::Success; }

void DomainModuleBase::reinit() {}

} // namespace Nebulite::Module::Base
