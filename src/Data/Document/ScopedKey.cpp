#include "Data/Document/ScopedKey.hpp"
#include "Data/Document/JsonScopeBase.hpp"

// OwnedScopedKey methods
namespace Nebulite::Data {

// construct from a view + suffix (runtime)
OwnedScopedKey::OwnedScopedKey(ScopedKey const& base, std::string_view suffix)
    : givenScope(base.givenScope), owned(base.key)
{
    owned.append(suffix);
}

OwnedScopedKey::OwnedScopedKey(std::string s, std::optional<std::string_view> const& scope)
    : givenScope(scope), owned(std::move(s)) {}

ScopedKey OwnedScopedKey::view() const & noexcept {
    return {givenScope, std::string_view(owned)};
}

OwnedScopedKey::operator ScopedKey() const & noexcept { return view(); }

OwnedScopedKey OwnedScopedKey::operator+(std::string_view const& suffix) const {
    return {owned + std::string(suffix), givenScope};
}

} // namespace Nebulite::Data

// ScopedKey methods
namespace Nebulite::Data {

std::string ScopedKey::full(JsonScopeBase const& scope) const {
    // The scope that this JsonScopeBase is allowed to use
    std::string const& allowedScope = scope.getScopePrefix();

    // See if we require a specific scope
    bool const requiresScope = givenScope.has_value();
    std::string fullKey;
    if (requiresScope) {
        // Ensure that the given scope lies within the allowed scope
        // E.g. givenScope = "module1.submodule." allowedScope = "module1." -> valid
        //      givenScope = "module2."           allowedScope = "module1." -> invalid, we are only allowed to use module1.*
        std::string const& given = std::string(*givenScope);
        if (!given.starts_with(allowedScope)) {
            std::string const msg =
                "ScopedKey scope mismatch: key '" + std::string(key) +
                "' was created with the given scope prefix '" + given +
                "' but was used in JsonScopeBase with prefix '" + allowedScope + "'.";
            throw std::invalid_argument(msg);
        }

        // Now we can safely use the given scope, as it lies within the allowed scope
        fullKey.reserve(given.size() + key.size());
        fullKey = given;
        fullKey.append(key);
        return fullKey;
    }
    fullKey.reserve(allowedScope.size() + key.size());
    fullKey = scope.getScopePrefix();
    fullKey.append(key);
    return fullKey;
}

OwnedScopedKey ScopedKey::operator+(std::string_view const& suffix) const {
    return OwnedScopedKey(*this, suffix);
}

} // namespace Nebulite::Data
