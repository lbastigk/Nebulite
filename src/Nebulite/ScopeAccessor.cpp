//------------------------------------------
// Includes

// Standard library
#include <optional>
#include <string>

// Nebulite
#include "Nebulite/Module/Base/RulesetModule.hpp"
#include "Nebulite/ScopeAccessor.hpp"

//------------------------------------------
namespace Nebulite {

ScopeAccessor::BaseAccessToken::BaseAccessToken() = default;

std::optional<std::string> const& ScopeAccessor::BaseAccessToken::getPrefix() const {
    return prefix;
}

ScopeAccessor::Full::Full() {
    prefix = ""; // Full access gets an empty prefix, meaning it can access the entire GlobalSpace document
}

ScopeAccessor::Full::~Full() = default;

// RulesetModule accessor

ScopeAccessor::RulesetModuleToken::RulesetModuleToken(Module::Base::RulesetModule const& rm) {
    prefix = rm.globalAccessScopePrefix;
}

} // namespace Nebulite
