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
      scopePrefix(ScopedKeyView(generatePrefix(prefix)).full(other)), // Generate full scoped prefix based on the other JsonScopeBase and the new prefix
      expressionRefs(make_array_with_arg<MappedOrderedDoublePointers, ORDERED_DOUBLE_POINTERS_MAPS>(*this))
{}

// Default constructor, we create a self-owned empty JSON document
JsonScopeBase::JsonScopeBase()
    : baseDocument(std::make_shared<JSON>()), scopePrefix(""),
      expressionRefs(make_array_with_arg<MappedOrderedDoublePointers, ORDERED_DOUBLE_POINTERS_MAPS>(*this))
{}

JsonScopeBase::~JsonScopeBase() = default;

//------------------------------------------
// Sharing a scope

JsonScopeBase& JsonScopeBase::shareScopeBase(std::string const& key) const {
    return baseDocument->shareManagedScopeBase(
        ScopedKey(key).view().full(*this)
    );
}

//------------------------------------------
// Getter

[[nodiscard]] std::optional<RjDirectAccess::simpleValue> JsonScopeBase::getVariant(ScopedKeyView const& key) const {
    return baseDocument->getVariant(key.full(*this));
}

[[nodiscard]] JSON JsonScopeBase::getSubDoc(ScopedKeyView const& key) const {
    return baseDocument->getSubDoc(key.full(*this));
}
[[nodiscard]] JSON JsonScopeBase::getSubDoc(ScopedKey const& key) const {
    return getSubDoc(key.view());
}

[[nodiscard]] double* JsonScopeBase::getStableDoublePointer(ScopedKeyView const& key) const {
    return baseDocument->getStableDoublePointer(key.full(*this));
}

//------------------------------------------
// Setter

void JsonScopeBase::setVariant(ScopedKeyView const& key, RjDirectAccess::simpleValue const& value) const {
    baseDocument->setVariant(key.full(*this), value);
}

void JsonScopeBase::setSubDoc(ScopedKeyView const& key, JSON const& subDoc) const {
    baseDocument->setSubDoc(key.full(*this), subDoc);
}

void JsonScopeBase::setSubDoc(ScopedKeyView const& key, JsonScopeBase const& subDoc) const {
    // Slightly more complicated: If we wish to set the sub-document from another JsonScopeBase,
    // we need to extract the underlying JSON document from it in the correct scope.
    JSON const subDocScope = subDoc.getSubDoc(ScopedKey(""));
    baseDocument->setSubDoc(key.full(*this), subDocScope);
}

void JsonScopeBase::setEmptyArray(ScopedKeyView const& key) const {
    baseDocument->setEmptyArray(key.full(*this));
}

//------------------------------------------
// Special sets for threadsafe maths operations

void JsonScopeBase::set_add(ScopedKeyView const& key, double const& val) const {
    baseDocument->set_add(key.full(*this), val);
}

void JsonScopeBase::set_multiply(ScopedKeyView const& key, double const& val) const {
    baseDocument->set_multiply(key.full(*this), val);
}

void JsonScopeBase::set_concat(ScopedKeyView const& key, std::string const& valStr) const {
    baseDocument->set_concat(key.full(*this), valStr);
}

//------------------------------------------
// Locking

[[nodiscard]] std::scoped_lock<std::recursive_mutex> JsonScopeBase::lock() const {
    return baseDocument->lock();
}

//------------------------------------------
// Key Types, Sizes

[[nodiscard]] KeyType JsonScopeBase::memberType(ScopedKeyView const& key) const {
    return baseDocument->memberType(key.full(*this));
}

[[nodiscard]] size_t JsonScopeBase::memberSize(ScopedKeyView const& key) const {
    return baseDocument->memberSize(key.full(*this));
}

void JsonScopeBase::removeKey(ScopedKeyView const& key) const {
    baseDocument->removeKey(key.full(*this));
}

//------------------------------------------
// Deserialize/Serialize

std::string JsonScopeBase::serialize() const {
    static ScopedKeyView constexpr key("");
    return baseDocument->serialize(key.full(*this));
}

std::string JsonScopeBase::serialize(ScopedKeyView const& key) const {
    return baseDocument->serialize(key.full(*this));
}

void JsonScopeBase::deserialize(std::string const& serialOrLink) {
    // No support for any tokens, just forward to baseDocument
    baseDocument->deserialize(serialOrLink);
}

} // namespace Nebulite::Data
