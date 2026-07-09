//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

// Nebulite
#include "Nebulite/Data/Document/ScopedKey.hpp"
#include "Nebulite/Data/Document/ScopedKeyView.hpp"

//------------------------------------------
// ScopedKey methods
namespace Nebulite::Data {

// construct from a view + suffix
ScopedKey::ScopedKey(ScopedKeyView const& base, std::string_view const suffix)
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

ScopedKey ScopedKey::operator+(std::string_view const suffix) const {
    return ScopedKey{givenScope, ScopedKeyView::combineKeys(owned, suffix)};
}

ScopedKey ScopedKey::nestKey(ScopedKey const& other) const {
    return this->view().nestKey(other);
}
ScopedKey ScopedKey::nestKey(ScopedKeyView const& other) const {
    return this->view().nestKey(other);
}

ScopedKey ScopedKey::addIndex(std::size_t const index) const {
    return this->view().addIndex(index);
}

ScopedKey ScopedKey::addMember(std::string_view const member) const {
    return this->view().addMember(member);
}

} // namespace Nebulite::Data
