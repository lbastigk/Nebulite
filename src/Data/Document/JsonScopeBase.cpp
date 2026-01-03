#include "Nebulite.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/ScopedKey.hpp"

#include <array>


// JsonScopeBase methods
namespace Nebulite::Data {
// Constructing a JsonScopeBase from a JSON document and a prefix
JsonScopeBase::JsonScopeBase(JSON& doc, std::string const& prefix)
    // create a non-owning shared_ptr to the provided JSON (no delete on destruction)
    : baseDocument(std::shared_ptr<JSON>(&doc, [](JSON*){})), scopePrefix(generatePrefix(prefix)),
      expressionRefs(make_array_with_arg<MappedOrderedDoublePointers, ORDERED_DOUBLE_POINTERS_MAPS>(*this))
{}

// Constructing a JsonScopeBase from another JsonScopeBase and a sub-prefix
JsonScopeBase::JsonScopeBase(JsonScopeBase const& other, std::string const& prefix)
    : baseDocument(other.baseDocument),
      scopePrefix(ScopedKey(generatePrefix(prefix)).full(other)), // Generate full scoped prefix based on the other JsonScopeBase and the new prefix
      expressionRefs(make_array_with_arg<MappedOrderedDoublePointers, ORDERED_DOUBLE_POINTERS_MAPS>(*this))
{}

// Default constructor, we create a self-owned empty JSON document
JsonScopeBase::JsonScopeBase()
    : baseDocument(std::make_shared<JSON>()), scopePrefix(""),
      expressionRefs(make_array_with_arg<MappedOrderedDoublePointers, ORDERED_DOUBLE_POINTERS_MAPS>(*this))
{}

/*

// --- Copy constructor
JsonScopeBase::JsonScopeBase(JsonScopeBase const& other)
    : baseDocument(other.baseDocument),
      scopePrefix(other.scopePrefix),
      expressionRefs(make_array_with_arg<MappedOrderedDoublePointers, ORDERED_DOUBLE_POINTERS_MAPS>(*this))
{}

// --- Move constructor
JsonScopeBase::JsonScopeBase(JsonScopeBase&& other) noexcept
    : baseDocument(std::move(other.baseDocument)),
      scopePrefix(std::move(other.scopePrefix)),
      expressionRefs(make_array_with_arg<MappedOrderedDoublePointers, ORDERED_DOUBLE_POINTERS_MAPS>(*this))
{}

// --- Copy assignment (copy-and-swap)
JsonScopeBase& JsonScopeBase::operator=(JsonScopeBase const& other) {
    if (this == &other) return *this;
    JsonScopeBase tmp(other);
    swap(tmp);
    return *this;
}

// --- Move assignment (copy-and-swap with moved temporary)
JsonScopeBase& JsonScopeBase::operator=(JsonScopeBase&& other) noexcept {
    if (this == &other) return *this;
    JsonScopeBase tmp(std::move(other));
    swap(tmp);
    return *this;
}

// --- swap helper
void JsonScopeBase::swap(JsonScopeBase& o) noexcept {
    std::swap(baseDocument, o.baseDocument);
    std::swap(scopePrefix, o.scopePrefix);
}

*/

JsonScopeBase::~JsonScopeBase() = default;

//------------------------------------------
// Sharing a scope

JsonScopeBase& JsonScopeBase::shareManagedScopeBase(std::string const& prefix) const {
    return shareScopeBase(ScopedKey(prefix));
}

JsonScopeBase& JsonScopeBase::shareScopeBase(ScopedKey const& key) const {
    return baseDocument->shareManagedScopeBase(key.full(*this));
}

//------------------------------------------
// Getter

[[nodiscard]] std::optional<RjDirectAccess::simpleValue> JsonScopeBase::getVariant(ScopedKey const& key) const {
    return baseDocument->getVariant(key.full(*this));
}

[[nodiscard]] JSON JsonScopeBase::getSubDoc(ScopedKey const& key) const {
    return baseDocument->getSubDoc(key.full(*this));
}

[[nodiscard]] double* JsonScopeBase::getStableDoublePointer(ScopedKey const& key) const {
    return baseDocument->getStableDoublePointer(key.full(*this));
}

//------------------------------------------
// Setter

void JsonScopeBase::setVariant(ScopedKey const& key, RjDirectAccess::simpleValue const& value) const {
    baseDocument->setVariant(key.full(*this), value);
}

void JsonScopeBase::setSubDoc(ScopedKey const& key, JSON& subDoc) const {
    baseDocument->setSubDoc(key.full(*this), subDoc);
}

void JsonScopeBase::setSubDoc(ScopedKey const& key, JsonScopeBase const& subDoc) const {
    // Slightly more complicated: If we wish to set the sub-document from another JsonScopeBase,
    // we need to extract the underlying JSON document from it in the correct scope.
    JSON subDocScope = subDoc.getSubDoc(ScopedKey(""));
    baseDocument->setSubDoc(key.full(*this), subDocScope);
}

void JsonScopeBase::setEmptyArray(ScopedKey const& key) const {
    baseDocument->setEmptyArray(key.full(*this));
}

//------------------------------------------
// Special sets for threadsafe maths operations

void JsonScopeBase::set_add(ScopedKey const& key, double const& val) const {
    baseDocument->set_add(key.full(*this), val);
}

void JsonScopeBase::set_multiply(ScopedKey const& key, double const& val) const {
    baseDocument->set_multiply(key.full(*this), val);
}

void JsonScopeBase::set_concat(ScopedKey const& key, std::string const& valStr) const {
    baseDocument->set_concat(key.full(*this), valStr);
}

//------------------------------------------
// Locking

[[nodiscard]] std::scoped_lock<std::recursive_mutex> JsonScopeBase::lock() const {
    return baseDocument->lock();
}

//------------------------------------------
// Key Types, Sizes

[[nodiscard]] KeyType JsonScopeBase::memberType(ScopedKey const& key) const {
    return baseDocument->memberType(key.full(*this));
}

[[nodiscard]] size_t JsonScopeBase::memberSize(ScopedKey const& key) const {
    return baseDocument->memberSize(key.full(*this));
}

void JsonScopeBase::removeKey(ScopedKey const& key) const {
    baseDocument->removeKey(key.full(*this));
}

//------------------------------------------
// Deserialize/Serialize

std::string JsonScopeBase::serialize() const {
    ScopedKey key("");
    return baseDocument->serialize(key.full(*this));
}

std::string JsonScopeBase::serialize(ScopedKey const& key) const {
    return baseDocument->serialize(key.full(*this));
}

void JsonScopeBase::deserialize(std::string const& serialOrLink) {
    // No support for any tokens, just forward to baseDocument
    baseDocument->deserialize(serialOrLink);
}

} // namespace Nebulite::Data
