//------------------------------------------
// Includes

// Standard library
#include <array>

// Nebulite
#include "Data/Document/JSON.hpp"
#include "Data/Document/ScopedKey.hpp"
#include "Utility/Generate.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Data {

// Constructing a JsonScope from a JSON document and a prefix
JsonScope::JsonScope(JSON& doc, std::optional<std::string> const& prefix)
    // create a non-owning shared_ptr to the provided JSON (no delete on destruction)
    : baseDocument(std::shared_ptr<JSON>(&doc, [](JSON*){})),
      scopePrefix(prefix.has_value() ? std::optional(generatePrefix(prefix.value())) : std::nullopt),
      odpCache(Utility::Generate::array<MappedOrderedCacheList, noLockArraySize>([this](size_t) {return MappedOrderedCacheList(*this);}))
{}

// Constructing a JsonScope from another JsonScope and a sub-prefix
JsonScope::JsonScope(JsonScope const& other, std::optional<std::string> const& prefix)
    : baseDocument(other.baseDocument),
      scopePrefix(prefix.has_value() ? std::optional(ScopedKeyView(generatePrefix(prefix.value())).full(other)) : std::nullopt), // Generate full scoped prefix based on the other JsonScope and the new prefix
      odpCache(Utility::Generate::array<MappedOrderedCacheList, noLockArraySize>([this](size_t){return MappedOrderedCacheList(*this);}))
{}

// Default constructor, we create a self-owned empty JSON document
JsonScope::JsonScope()
    : baseDocument(std::make_shared<JSON>()),
      scopePrefix(""),
      odpCache(Utility::Generate::array<MappedOrderedCacheList, noLockArraySize>([this](size_t) {return MappedOrderedCacheList(*this);}))
{}

JsonScope::~JsonScope() = default;

//------------------------------------------
// Sharing a scope

JsonScope& JsonScope::shareScope(ScopedKeyView const& key) const {
    return baseDocument->shareManagedScopeBase(
        key.full(*this)
    );
}

JsonScope& JsonScope::shareScope(std::string const& key) const {
    return baseDocument->shareManagedScopeBase(
        ScopedKey(key).view().full(*this)
    );
}

JsonScope& JsonScope::shareDummyScopeBase() {
    if (!scopePrefix.has_value() || !baseDocument) {
        return *this; // If this is already a dummy scope, return itself
    }

    // Technically, key name does not matter.
    // But if the access control ever fails, at least we only access a known dummy scope,
    // instead of the entire scope.
    return baseDocument->getDummyScopeBase();
}

//------------------------------------------
// Getter

[[nodiscard]] std::expected<RjDirectAccess::simpleValue, SimpleValueRetrievalError> JsonScope::getVariant(ScopedKeyView const& key) const {
    return baseDocument->getVariant(key.full(*this));
}

[[nodiscard]] JSON JsonScope::getSubDoc(ScopedKeyView const& key) const {
    return baseDocument->getSubDoc(key.full(*this));
}
[[nodiscard]] JSON JsonScope::getSubDoc(ScopedKey const& key) const {
    return getSubDoc(key.view());
}

[[nodiscard]] double* JsonScope::getStableDoublePointer(ScopedKeyView const& key) const {
    return baseDocument->getStableDoublePointer(key.full(*this));
}

//------------------------------------------
// Setter

void JsonScope::setVariant(ScopedKeyView const& key, RjDirectAccess::simpleValue const& value){
    doc().setVariant(key.full(*this), value);
}

void JsonScope::setSubDoc(ScopedKeyView const& key, JSON const& subDoc){
    doc().setSubDoc(key.full(*this), subDoc);
}

void JsonScope::setSubDoc(ScopedKeyView const& key, JsonScope const& subDoc){
    // Slightly more complicated: If we wish to set the sub-document from another JsonScope,
    // we need to extract the underlying JSON document from it in the correct scope.
    JSON const subDocScope = subDoc.getSubDoc(ScopedKey(""));
    doc().setSubDoc(key.full(*this), subDocScope);
}

void JsonScope::setEmptyArray(ScopedKeyView const& key){
    doc().setEmptyArray(key.full(*this));
}

//------------------------------------------
// Special sets for threadsafe maths operations

void JsonScope::set_add(ScopedKeyView const& key, double const& val){
    doc().set_add(key.full(*this), val);
}

void JsonScope::set_multiply(ScopedKeyView const& key, double const& val){
    doc().set_multiply(key.full(*this), val);
}

void JsonScope::set_concat(ScopedKeyView const& key, std::string const& valStr){
    doc().set_concat(key.full(*this), valStr);
}

//------------------------------------------
// Locking

[[nodiscard]] std::unique_lock<std::recursive_mutex> JsonScope::lock() const {
    return baseDocument->lock();
}

//------------------------------------------
// Key Types, Sizes

[[nodiscard]] KeyType JsonScope::memberType(ScopedKeyView const& key) const {
    return baseDocument->memberType(key.full(*this));
}

[[nodiscard]] std::string JsonScope::memberTypeString(ScopedKeyView const& key) const {
    return baseDocument->memberTypeString(key.full(*this));
}

[[nodiscard]] size_t JsonScope::memberSize(ScopedKeyView const& key) const {
    return baseDocument->memberSize(key.full(*this));
}

void JsonScope::removeMember(ScopedKeyView const& key){
    doc().removeMember(key.full(*this));
}

void JsonScope::moveMember(ScopedKeyView const& fromKey, ScopedKeyView const& toKey){
    doc().moveMember(fromKey.full(*this), toKey.full(*this));
}

void JsonScope::copyMember(ScopedKeyView const& fromKey, ScopedKeyView const& toKey){
    doc().copyMember(fromKey.full(*this), toKey.full(*this));
}

std::vector<ScopedKey> JsonScope::listAvailableKeys(ScopedKeyView const& key) const {
    std::vector<std::string> const keys = baseDocument->listAvailableKeys(key.full(*this));
    std::vector<ScopedKey> scopedKeys;
    scopedKeys.reserve(keys.size());
    for (auto const& k : keys) {
        if (key.toString().ends_with('.') || key.toString().empty()) {
            scopedKeys.emplace_back(key + k);
        } else {
            scopedKeys.emplace_back(key + "." + k);
        }
    }
    return scopedKeys;
}

std::vector<JsonScope::MemberAndKey> JsonScope::listAvailableMembersAndKeys(ScopedKeyView const& key) const {
    std::vector<std::string> const keys = baseDocument->listAvailableKeys(key.full(*this));
    std::vector<MemberAndKey> scopedKeys;
    scopedKeys.reserve(keys.size());
    for (auto const& k : keys) {
        if (key.toString().ends_with('.') || key.toString().empty()) {
            scopedKeys.emplace_back(MemberAndKey{k, key + k});
        } else {
            scopedKeys.emplace_back(MemberAndKey{k, key + "." + k});
        }
    }
    return scopedKeys;
}

//------------------------------------------
// Deserialize/Serialize

std::string JsonScope::serialize() const {
    static ScopedKeyView constexpr key("");
    return baseDocument->serialize(key.full(*this));
}

std::string JsonScope::serialize(ScopedKeyView const& key) const {
    return baseDocument->serialize(key.full(*this));
}

void JsonScope::deserialize(std::string const& serialOrLink) {
    JSON tempDoc;
    tempDoc.deserialize(serialOrLink);
    static ScopedKeyView constexpr key("");
    std::string const fullKey = key.full(*this);
    doc().setSubDoc(fullKey, tempDoc);
}

//------------------------------------------
// Transform

bool JsonScope::transform(std::span<std::string const> const& args){
    return JsonRvalueTransformer::instance().parseSingleTransformation(args, this);
}

//------------------------------------------
// Access test

void JsonScope::assertAccess(ScopedKeyView const& key) const{
    (void)key.full(*this); // Just generate full key to test access
}

} // namespace Nebulite::Data
