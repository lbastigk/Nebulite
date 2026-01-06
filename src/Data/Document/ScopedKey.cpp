#include "Data/Document/ScopedKey.hpp"
#include "Data/Document/JsonScopeBase.hpp"

// ScopedKey methods
namespace Nebulite::Data {

// construct from a view + suffix (runtime)
ScopedKey::ScopedKey(ScopedKeyView const& base, std::string_view const& suffix)
    : givenScope(base.givenScope), owned(base.key)
{
    owned.append(suffix);
}

ScopedKey::ScopedKey(std::string s, std::optional<std::string_view> const& scope)
    : givenScope(scope), owned(std::move(s)) {}

ScopedKeyView ScopedKey::view() const & noexcept {
    return {givenScope, std::string_view(owned)};
}

ScopedKey ScopedKey::operator+(std::string_view const& suffix) const {
    return ScopedKey{owned + std::string(suffix), givenScope};
}

} // namespace Nebulite::Data

// ScopedKeyView methods
namespace Nebulite::Data {

std::string ScopedKeyView::full(JsonScopeBase const& scope) const {
    // The scope that this JsonScopeBase is allowed to use
    std::string const& allowedScope = scope.getScopePrefix();

    // See if we require a specific scope
    bool const requiresScope = givenScope.has_value();
    std::string fullKey;
    if (requiresScope) {
        // Ensure that the given scope lies within the allowed scope
        // E.g. givenScope = "module1.submodule." allowedScope = "module1." -> valid
        //      givenScope = "module2."           allowedScope = "module1." -> invalid, we are only allowed to use module1.*
        auto const& given = std::string(*givenScope);
        if (!given.starts_with(allowedScope)) {
            std::string const msg =
                "ScopedKey scope mismatch:" +
                ( key.empty() ? " an empty key" : " key '" + std::string(key) + "'" ) +
                " was created with the given scope prefix '" + given + "'" +
                " but was used in JsonScopeBase with prefix '" + allowedScope + "'.";
            throw std::invalid_argument(msg);
        }
        // Now we can safely use the given scope, as it lies within the allowed scope
        // Reserve space
        fullKey.reserve(given.size() + key.size());
        fullKey = given; // start with the given scope

        // Special cases to consider if our scope ends with a dot
        if (!fullKey.empty() && fullKey.back() == '.') {
            // NOLINTNEXTLINE
            if (key.empty()) {
                // empty key, remove trailing dot
                fullKey.pop_back();
            }
            else if (key.front() == '[') {
                // array index access, remove trailing dot
                fullKey.pop_back();
            }
        }

        // Combine and return
        // We assume there is no issue with key itself regarding trailing/leading dots etc.
        return fullKey.append(key);
    }
    fullKey.reserve(allowedScope.size() + key.size());
    fullKey = scope.getScopePrefix();
    return fullKey.append(key);
}

ScopedKey ScopedKeyView::operator+(std::string_view const& suffix) const {
    return {*this, suffix};
}

} // namespace Nebulite::Data
