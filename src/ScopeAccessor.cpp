#include "ScopeAccessor.hpp"

#include "Interaction/Execution/DomainModule.hpp"

namespace Nebulite {

// DomainModule accessors

ScopeAccessor::DomainModuleToken::GlobalSpace::GlobalSpace(Interaction::Execution::DomainModule<Core::GlobalSpace> const& dm) {
    prefix = "" + dm.moduleScope.getScopePrefix();
}

ScopeAccessor::DomainModuleToken::RenderObject::RenderObject(Interaction::Execution::DomainModule<Core::RenderObject> const& dm) {
    prefix = "providedScope.domainModule.renderObject."  + dm.moduleScope.getScopePrefix();
}

ScopeAccessor::DomainModuleToken::JsonScope::JsonScope(Interaction::Execution::DomainModule<Core::JsonScope> const& dm) {
    prefix = "providedScope.domainModule.jsonScope." + dm.moduleScope.getScopePrefix();
}

// RulesetModule accessor

ScopeAccessor::RulesetToken::RulesetModule::RulesetModule(Interaction::Rules::RulesetModule const& rm) {
    (void)rm; // TODO: add getScopePrefix() to RulesetModule later on
    prefix = ""; // RulesetModules get full access for now
}

} // namespace Nebulite
