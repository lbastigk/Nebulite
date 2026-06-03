//------------------------------------------
// Includes

// Standard library
#include <cstddef> // NOLINT
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Data/Document/ScopedKey.hpp"

//------------------------------------------
// ScopedKey methods
namespace Nebulite::Data {

// construct from a view + suffix
ScopedKey::ScopedKey(ScopedKeyView const& base, std::string_view const& suffix)
    : givenScope(base.givenScope), owned(base.key) {
    owned = ScopedKeyView::combineKeys(owned, suffix);
}

ScopedKey::ScopedKey(std::optional<std::string_view> const& scope, std::string suffix)
    : givenScope(scope), owned(std::move(suffix)) {}

ScopedKey::ScopedKey(std::string_view const suffix)
    : owned(std::string(suffix)) {}

std::string ScopedKey::toString() const {
    return view().toString();
}

ScopedKeyView ScopedKey::view() const & noexcept {
    return {givenScope, std::string_view(owned)};
}

ScopedKey ScopedKey::operator+(std::string_view const& suffix) const {
    return ScopedKey{givenScope, ScopedKeyView::combineKeys(owned, suffix)};
}

ScopedKey ScopedKey::nestKey(ScopedKey const& other) const {
    return this->view().nestKey(other);
}
ScopedKey ScopedKey::nestKey(ScopedKeyView const& other) const {
    return this->view().nestKey(other);
}

ScopedKey ScopedKey::addIndex(size_t const& index) const {
    return this->view().addIndex(index);
}

ScopedKey ScopedKey::addMember(std::string_view const& member) const {
    return this->view().addMember(member);
}

} // namespace Nebulite::Data

//------------------------------------------
// ScopedKeyView methods
namespace Nebulite::Data {

std::string ScopedKeyView::buildKey() const {
    // See if we require a specific scope
    if (givenScope.has_value()) {
        return combineKeys(givenScope.value(), std::string(key));
    }
    return std::string(key);
}

std::string ScopedKeyView::combineKeys(std::string_view const& key1, std::string_view key2) {
    std::string fullKey;
    fullKey.reserve(key1.size() + key2.size());

    // start with the given scope + a dot, if it doesn't have one.
    fullKey = key1;
    if (!key1.empty() && !key1.ends_with(JSON::SpecialCharacter::dot)) {
        fullKey += JSON::SpecialCharacter::dot;
    }

    // remove trailing dot from key2
    if (key2.starts_with(JSON::SpecialCharacter::dot)) {
        key2.remove_prefix(1);
    }

    // remove trailing dot from key1
    if (!fullKey.empty()) {
        if (key2.empty() || key2.front() == JSON::SpecialCharacter::arrayOpen) {
            fullKey.pop_back();
        }
    }

    // Remove dot at the beginning
    if (fullKey.starts_with(JSON::SpecialCharacter::dot)) {
        fullKey.erase(0, 1);
    }

    // Append second key
    return fullKey.append(key2);
}

std::string ScopedKeyView::full(JsonScope const& scope) const {
    // The scope that this JsonScope is allowed to use
    std::string const& allowedScope = scope.getScopePrefix();

    // See if we require a specific scope
    if (givenScope.has_value()) {
        // Ensure that the given scope lies within the allowed scope
        // E.g. givenScope = "module1.submodule." allowedScope = "module1." -> valid
        //      givenScope = "module2."           allowedScope = "module1." -> invalid, we are only allowed to use module1.*
        if (!givenScope.value().starts_with(allowedScope)) {
            std::string const msg =
                "ScopedKey scope mismatch:" +
                ( key.empty() ? " an empty key" : " key '" + std::string(key) + "'" ) +
                " was created with the given scope prefix '" + givenScope.value() + "'" +
                " but was used in JsonScope with prefix '" + allowedScope + "'.";
            throw std::invalid_argument(msg);
        }
        return buildKey();
    }
    return combineKeys(allowedScope, key);
}

ScopedKey ScopedKeyView::operator+(std::string_view const& suffix) const {
    return {*this, suffix};
}

ScopedKey ScopedKeyView::nestKey(ScopedKey const& other) const {
    auto const suffix = other.view().buildKey();
    return ScopedKey(givenScope, combineKeys(key,suffix));
}

ScopedKey ScopedKeyView::nestKey(ScopedKeyView const& other) const {
    auto const suffix = other.buildKey();
    return ScopedKey(givenScope, combineKeys(key,suffix));
}

ScopedKey ScopedKeyView::addIndex(size_t const& index) const {
    std::string const arrayIndex = "[" + std::to_string(index) + "]";
    return *this + arrayIndex;
}

ScopedKey ScopedKeyView::addMember(std::string_view const& member) const {
    return *this + member;
}

} // namespace Nebulite::Data
