//------------------------------------------
// Includes

// Standard library
#include <string>

// Nebulite
#include "Nebulite/ScopeAccessor.hpp"

//------------------------------------------
namespace Nebulite {

ScopeAccessor::BaseAccessToken::BaseAccessToken() = default;

std::string const& ScopeAccessor::BaseAccessToken::getPrefix() const {
    return prefix;
}

ScopeAccessor::Full::Full() {
    prefix = ""; // Full access gets an empty prefix, meaning it can access the entire GlobalSpace document
}

ScopeAccessor::Full::~Full() = default;

// RulesetModule accessor

ScopeAccessor::RulesetModuleToken::RulesetModuleToken(Module::Base::RulesetModule const& rm) {
    (void)rm; // TODO: add getScopePrefix() to RulesetModule later on
    prefix = ""; // RulesetModules get full access for now
}

} // namespace Nebulite
