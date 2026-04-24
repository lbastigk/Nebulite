#include "Interaction/Execution/DomainModule.hpp"
#include "ScopeAccessor.hpp"

namespace Nebulite {

// RulesetModule accessor

ScopeAccessor::RulesetModuleToken::RulesetModuleToken(Interaction::Rules::RulesetModule const& rm) {
    (void)rm; // TODO: add getScopePrefix() to RulesetModule later on
    prefix = ""; // RulesetModules get full access for now
}

} // namespace Nebulite
