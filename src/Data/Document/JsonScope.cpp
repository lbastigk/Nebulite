//------------------------------------------
// Includes

// Standard library
#include <complex>
#include <cstddef>
#include <cstdint> // NOLINT
#include <expected>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/JSON.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Data/Document/RjDirectAccess.hpp"
#include "Nebulite/Data/Document/ScopedKey.hpp"
#include "Nebulite/Data/Document/SimpleValueError.hpp"
#include "Nebulite/Data/MappedOrderedCacheList.hpp"
#include "Nebulite/Utility/Coordination/IdGenerator.hpp"
#include "Nebulite/Utility/Generate.hpp"

//------------------------------------------
namespace Nebulite::Data {

// Constructing a JsonScope from a JSON document and a prefix
JsonScope::JsonScope(JSON& doc, std::optional<std::string> const& prefix)
    // create a non-owning shared_ptr to the provided JSON (no delete on destruction)
    : baseDocument(std::shared_ptr<JSON>(&doc, [](JSON*){}))
    , scopePrefix(prefix.has_value() ? std::optional(generatePrefix(prefix.value())) : std::nullopt)
    , odpCache(Utility::Generate::array<MappedOrderedCacheList, cacheLookupThreadCount>([this](std::size_t) {return MappedOrderedCacheList(*this);}))
{}

// Constructing a JsonScope from another JsonScope and a sub-prefix
JsonScope::JsonScope(JsonScope const& other, std::optional<std::string> const& prefix)
    : baseDocument(other.baseDocument)
    , scopePrefix(prefix.has_value() ? std::optional(ScopedKeyView(generatePrefix(prefix.value())).full(other)) : std::nullopt) // Generate full scoped prefix based on the other JsonScope and the new prefix
    , odpCache(Utility::Generate::array<MappedOrderedCacheList, cacheLookupThreadCount>([this](std::size_t){return MappedOrderedCacheList(*this);}))
{}

// Default constructor, we create a self-owned empty JSON document
JsonScope::JsonScope()
    : baseDocument(std::make_shared<JSON>())
    , scopePrefix("")
    , odpCache(Utility::Generate::array<MappedOrderedCacheList, cacheLookupThreadCount>([this](std::size_t) {return MappedOrderedCacheList(*this);}))
{}

JsonScope::~JsonScope() = default;

//------------------------------------------
// Sharing a scope

JsonScope& JsonScope::shareScope(ScopedKeyView const& key) const {
    return baseDocument->shareManagedScope(
        key.full(*this)
    );
}

JsonScope& JsonScope::shareScope(std::string const& key) const {
    return baseDocument->shareManagedScope(
        ScopedKey(key).view().full(*this)
    );
}

JsonScope& JsonScope::shareDummyScope() {
    if (!scopePrefix.has_value() || !baseDocument) {
        return *this; // If this is already a dummy scope, return itself
    }

    // Technically, key name does not matter.
    // But if the access control ever fails, at least we only access a known dummy scope,
    // instead of the entire scope.
    return baseDocument->getDummyScope();
}

//------------------------------------------
// Getter

std::expected<RjDirectAccess::simpleValue, SimpleValueRetrievalError> JsonScope::getVariant(ScopedKeyView const& key) const {
    return baseDocument->getVariant(key.full(*this));
}

JSON JsonScope::getSubDoc(ScopedKeyView const& key) const {
    return baseDocument->getSubDoc(key.full(*this));
}
JSON JsonScope::getSubDoc(ScopedKey const& key) const {
    return getSubDoc(key.view());
}

double* JsonScope::getStableDoublePointer(ScopedKeyView const& key) const {
    return baseDocument->getStableDoublePointer(key.full(*this));
}

std::optional<std::complex<double>> JsonScope::getComplex(ScopedKeyView const& key) const {
    auto realPart = baseDocument->get<double>(key.addMember(complexRe).view().full(*this));
    auto imagPart = baseDocument->get<double>(key.addMember(complexIm).view().full(*this));

    if (realPart.has_value() && imagPart.has_value()) {
        return {std::complex(realPart.value(), imagPart.value())};
    }

    return std::nullopt;
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

void JsonScope::setComplex(ScopedKeyView const& key, std::complex<double> const& value){
    baseDocument->removeMember(key.full(*this)); // Remove any existing member to avoid type conflicts
    baseDocument->set<double>(key.addMember(complexRe).view().full(*this), value.real());
    baseDocument->set<double>(key.addMember(complexIm).view().full(*this), value.imag());
}

//------------------------------------------
// Special sets for threadsafe maths operations

void JsonScope::set_add(ScopedKeyView const& key, double const val){
    doc().set_add(key.full(*this), val);
}

void JsonScope::set_add(ScopedKeyView const& key, std::int64_t const val){
    doc().set_add(key.full(*this), val);
}

void JsonScope::set_multiply(ScopedKeyView const& key, double const val){
    doc().set_multiply(key.full(*this), val);
}

void JsonScope::set_multiply(ScopedKeyView const& key, std::int64_t const val){
    doc().set_multiply(key.full(*this), val);
}

void JsonScope::set_concat(ScopedKeyView const& key, std::string const& valStr){
    doc().set_concat(key.full(*this), valStr);
}

//------------------------------------------
// Locking

std::unique_lock<std::recursive_mutex> JsonScope::lock() const {
    return baseDocument->lock();
}

//------------------------------------------
// Ordered cache list related

std::size_t JsonScope::assignCacheLookupIndex() {
    static auto indexCounter = Utility::Coordination::IdGenerator::atomicIncrementIdGenerator();
    thread_local std::size_t const threadIndex = indexCounter();
    return threadIndex;
}

//------------------------------------------
// Key Types, Sizes

KeyType JsonScope::memberType(ScopedKeyView const& key) const {
    return baseDocument->memberType(key.full(*this));
}

std::string JsonScope::memberTypeString(ScopedKeyView const& key) const {
    return baseDocument->memberTypeString(key.full(*this));
}

std::size_t JsonScope::memberSize(ScopedKeyView const& key) const {
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
        scopedKeys.emplace_back(key.addMember(k));
    }
    return scopedKeys;
}

std::vector<JsonScope::MemberAndKey> JsonScope::listAvailableMembersAndKeys(ScopedKeyView const& key) const {
    std::vector<std::string> const keys = baseDocument->listAvailableKeys(key.full(*this));
    std::vector<MemberAndKey> scopedKeys;
    scopedKeys.reserve(keys.size());
    for (auto const& k : keys) {
        scopedKeys.emplace_back(
            MemberAndKey{
                .member=k,
                .key=key.addMember(k)
            }
        );
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

void JsonScope::deserialize(std::string_view const serialOrLink) {
    JSON tempDoc;
    tempDoc.deserialize(serialOrLink);
    static ScopedKeyView constexpr key("");
    std::string const fullKey = key.full(*this);
    doc().setSubDoc(fullKey, tempDoc);
}

//------------------------------------------
// Transform

bool JsonScope::transform(std::span<std::string_view const> const& args){
    return JsonTransformer::instance().parseSingleTransformation(args, *this);
}

//------------------------------------------
// Access test

void JsonScope::assertAccess(ScopedKeyView const& key) const{
    (void)key.full(*this); // Just generate full key to test access
}

} // namespace Nebulite::Data
