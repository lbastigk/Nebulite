#include "Data/Document/JsonScope.hpp"
#include "DomainModule/Initializer.hpp"

namespace Nebulite::Data {

// --- Copy constructor
JsonScope::JsonScope(JsonScope const& other)
    : Domain(this->getName(), *this, *this),
      baseDocument(other.baseDocument),
      scopePrefix(other.scopePrefix)
{
    // If you need to re-register modules / reinit state after copy, do it here:
    reinitModules();
}

// --- Move constructor
JsonScope::JsonScope(JsonScope&& other) noexcept
    : Domain(this->getName(), *this, *this),
      baseDocument(std::move(other.baseDocument)),
      scopePrefix(std::move(other.scopePrefix))
{}

// --- Copy assignment (copy-and-swap)
JsonScope& JsonScope::operator=(JsonScope const& other) {
    if (this == &other) return *this;
    JsonScope tmp(other);
    swap(tmp);
    return *this;
}

// --- Move assignment (copy-and-swap with moved temporary)
JsonScope& JsonScope::operator=(JsonScope&& other) noexcept {
    if (this == &other) return *this;
    JsonScope tmp(std::move(other));
    swap(tmp);
    return *this;
}

// --- swap helper
void JsonScope::swap(JsonScope& o) noexcept {
    std::swap(baseDocument, o.baseDocument);
    std::swap(scopePrefix, o.scopePrefix);
}

JsonScope::~JsonScope() = default;

} // namespace Nebulite::Data
