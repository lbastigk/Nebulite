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
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/Json.hpp"
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
JsonScope::JsonScope(Json& doc, std::optional<std::string> const& prefix)
    // create a non-owning shared_ptr to the provided JSON (no delete on destruction)
    : baseDocument(std::shared_ptr<Json>(&doc, [](Json*){}))
    , scopePrefix(generateScopePrefix(doc, prefix))
    , odpCache(Utility::Generate::array<MappedOrderedCacheList, cacheLookupThreadCount>([this](std::size_t) {return MappedOrderedCacheList(*this);}))
{}

// Constructing a JsonScope from another JsonScope and a sub-prefix
JsonScope::JsonScope(JsonScope const& other, std::optional<std::string> const& prefix)
    : baseDocument(other.baseDocument)
    , scopePrefix(generateScopePrefix(other, prefix))
    , odpCache(Utility::Generate::array<MappedOrderedCacheList, cacheLookupThreadCount>([this](std::size_t) {return MappedOrderedCacheList(*this);}))
{}

// Default constructor, we create a self-owned empty JSON document
JsonScope::JsonScope()
    : baseDocument(std::make_shared<Json>())
    , scopePrefix("")
    , odpCache(Utility::Generate::array<MappedOrderedCacheList, cacheLookupThreadCount>([this](std::size_t) {return MappedOrderedCacheList(*this);}))
{}

JsonScope::~JsonScope() = default;

//------------------------------------------
// Private helpers

std::string JsonScope::generatePrefix(std::string const& givenPrefix) {
    std::string fullPrefix = givenPrefix;
    if (!fullPrefix.empty() && !fullPrefix.ends_with('.')) fullPrefix += '.';
    return fullPrefix;
}

//------------------------------------------
// Prefix

std::string const& JsonScope::getScopePrefix() const {
    if (!scopePrefix.has_value()) {
        throw std::runtime_error("JsonScope: Access not granted. Attempted to get scope prefix of a dummy scope. Did you mean to use the caller's scope?");
    }
    return scopePrefix.value();
}

ScopedKeyView JsonScope::getRootScope() const {
    return ScopedKeyView{getScopePrefix(), ""};
}

//------------------------------------------
// Sharing a scope

JsonScope& JsonScope::shareScope(ScopedKeyView const& key) const {
    return baseDocument->shareManagedScope(
        key.full(*this)
    );
}

JsonScope& JsonScope::shareScope(ScopedKey const& key) const {
    return shareScope(key.view());
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

std::expected<RjDirectAccess::SimpleValue, SimpleValueRetrievalError> JsonScope::getVariant(ScopedKeyView const& key) const {
    return baseDocument->getVariant(key.full(*this));
}

std::expected<RjDirectAccess::SimpleValue, SimpleValueRetrievalError> JsonScope::getVariant(ScopedKey const& key) const {
    return getVariant(key.view());
}

Json JsonScope::getSubDoc(ScopedKeyView const& key) const {
    return baseDocument->getSubDoc(key.full(*this));
}
Json JsonScope::getSubDoc(ScopedKey const& key) const {
    return getSubDoc(key.view());
}

double* JsonScope::getStableDoublePointer(ScopedKeyView const& key) const {
    return baseDocument->getStableDoublePointer(key.full(*this));
}

double* JsonScope::getStableDoublePointer(ScopedKey const& key) const {
    return getStableDoublePointer(key.view());
}

std::optional<std::complex<double>> JsonScope::getComplex(ScopedKeyView const& key) const {
    if (auto const [realPart, imagPart] = getMultiple<double>(key.addMember(complexRe).view(), key.addMember(complexIm).view()); realPart.has_value() && imagPart.has_value()) {
        return {std::complex(realPart.value(), imagPart.value())};
    }
    return std::nullopt;
}

std::optional<std::complex<double>> JsonScope::getComplex(ScopedKey const& key) const {
    return getComplex(key.view());
}

//------------------------------------------
// Setter

void JsonScope::setVariant(ScopedKeyView const& key, RjDirectAccess::SimpleValue const& value){
    doc().setVariant(key.full(*this), value);
}

void JsonScope::setVariant(ScopedKey const& key, RjDirectAccess::SimpleValue const& value) {
    setVariant(key.view(), value);
}

void JsonScope::setSubDoc(ScopedKeyView const& key, Json const& subDoc){
    doc().setSubDoc(key.full(*this), subDoc);
}

void JsonScope::setSubDoc(ScopedKey const& key, Json const& subDoc) {
    setSubDoc(key.view(), subDoc);
}

void JsonScope::setSubDoc(ScopedKeyView const& key, JsonScope const& subDoc){
    // Slightly more complicated: If we wish to set the sub-document from another JsonScope,
    // we need to extract the underlying JSON document from it in the correct scope.
    Json const subDocScope = subDoc.getSubDoc(ScopedKey(""));
    doc().setSubDoc(key.full(*this), subDocScope);
}

void JsonScope::setSubDoc(ScopedKey const& key, JsonScope const& subDoc) {
    setSubDoc(key.view(), subDoc);
}

void JsonScope::setEmptyArray(ScopedKeyView const& key){
    doc().setEmptyArray(key.full(*this));
}

void JsonScope::setEmptyArray(ScopedKey const& key) {
    setEmptyArray(key.view());
}

void JsonScope::setComplex(ScopedKeyView const& key, std::complex<double> const& value){
    doc().removeMember(key.full(*this)); // Remove any existing member to avoid type conflicts
    doc().set<double>(key.addMember(complexRe).view().full(*this), value.real());
    doc().set<double>(key.addMember(complexIm).view().full(*this), value.imag());
}

void JsonScope::setComplex(ScopedKey const& key, std::complex<double> const& value) {
    setComplex(key.view(), value);
}

void JsonScope::setEmptyObject(ScopedKeyView const& key){
    doc().setEmptyObject(key.full(*this));
}

void JsonScope::setEmptyObject(ScopedKey const& key) {
    setEmptyObject(key.view());
}

//------------------------------------------
// Special sets for threadsafe maths operations

void JsonScope::setAdditive(ScopedKeyView const& key, double const val){
    doc().setAdditive(key.full(*this), val);
}

void JsonScope::setAdditive(ScopedKey const& key, double const val) {
    setAdditive(key.view(), val);
}

void JsonScope::setAdditive(ScopedKeyView const& key, std::int64_t const val){
    doc().setAdditive(key.full(*this), val);
}

void JsonScope::setAdditive(ScopedKey const& key, std::int64_t const val) {
    setAdditive(key.view(), val);
}

void JsonScope::setMultiplicative(ScopedKeyView const& key, double const val){
    doc().setMultiplicative(key.full(*this), val);
}

void JsonScope::setMultiplicative(ScopedKey const& key, double const val) {
    setMultiplicative(key.view(), val);
}

void JsonScope::setMultiplicative(ScopedKeyView const& key, std::int64_t const val){
    doc().setMultiplicative(key.full(*this), val);
}

void JsonScope::setMultiplicative(ScopedKey const& key, std::int64_t const val) {
    setMultiplicative(key.view(), val);
}

void JsonScope::setConcatenative(ScopedKeyView const& key, std::string const& valStr){
    doc().setConcatenative(key.full(*this), valStr);
}

void JsonScope::setConcatenative(ScopedKey const& key, std::string const& valStr) {
    setConcatenative(key.view(), valStr);
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

KeyType JsonScope::memberType(ScopedKey const& key) const {
    return memberType(key.view());
}

std::string JsonScope::memberTypeString(ScopedKeyView const& key) const {
    return baseDocument->memberTypeString(key.full(*this));
}

std::string JsonScope::memberTypeString(ScopedKey const& key) const {
    return memberTypeString(key.view());
}

std::size_t JsonScope::memberSize(ScopedKeyView const& key) const {
    return baseDocument->memberSize(key.full(*this));
}

std::size_t JsonScope::memberSize(ScopedKey const& key) const {
    return memberSize(key.view());
}

//------------------------------------------
// Member manipulation

void JsonScope::removeMember(ScopedKeyView const& key){
    doc().removeMember(key.full(*this));
}

void JsonScope::removeMember(ScopedKey const& key) {
    removeMember(key.view());
}

void JsonScope::moveMember(ScopedKeyView const& fromKey, ScopedKeyView const& toKey){
    doc().moveMember(fromKey.full(*this), toKey.full(*this));
}

void JsonScope::moveMember(ScopedKey const& fromKey, ScopedKey const& toKey) {
    moveMember(fromKey.view(), toKey.view());
}

void JsonScope::moveMember(ScopedKeyView const& fromKey, ScopedKey const& toKey) {
    moveMember(fromKey, toKey.view());
}

void JsonScope::moveMember(ScopedKey const& fromKey, ScopedKeyView const& toKey) {
    moveMember(fromKey.view(), toKey);
}

void JsonScope::copyMember(ScopedKeyView const& fromKey, ScopedKeyView const& toKey){
    doc().copyMember(fromKey.full(*this), toKey.full(*this));
}

void JsonScope::copyMember(ScopedKey const& fromKey, ScopedKey const& toKey) {
    copyMember(fromKey.view(), toKey.view());
}

void JsonScope::copyMember(ScopedKeyView const& fromKey, ScopedKey const& toKey) {
    copyMember(fromKey, toKey.view());
}

void JsonScope::copyMember(ScopedKey const& fromKey, ScopedKeyView const& toKey) {
    copyMember(fromKey.view(), toKey);
}

//------------------------------------------
// Member listing

std::vector<ScopedKey> JsonScope::listAvailableKeys(ScopedKeyView const& key) const {
    return listAvailableMembers(key)
        | std::views::transform([&key](std::string const& member) {
            return key.addMember(member);
        })
        | std::ranges::to<std::vector<ScopedKey>>();
}

std::vector<ScopedKey> JsonScope::listAvailableKeys(ScopedKey const& key) const {
    return listAvailableKeys(key.view());
}

std::vector<JsonScope::MemberAndKey> JsonScope::listAvailableMembersAndKeys(ScopedKeyView const& key) const {
    return listAvailableMembers(key)
        | std::views::transform([&key](std::string const& member) {
            return MemberAndKey{
                .member=member,
                .key=key.addMember(member),
            };
        })
        | std::ranges::to<std::vector<MemberAndKey>>();
}

std::vector<JsonScope::MemberAndKey> JsonScope::listAvailableMembersAndKeys(ScopedKey const& key) const {
    return listAvailableMembersAndKeys(key.view());
}

std::vector<std::string> JsonScope::listAvailableMembers(ScopedKeyView const& key) const {
    return baseDocument->listAvailableMembers(key.full(*this));
}

std::vector<std::string> JsonScope::listAvailableMembers(ScopedKey const& key) const {
    return listAvailableMembers(key.view());
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

std::string JsonScope::serialize(ScopedKey const& key) const {
    return serialize(key.view());
}

void JsonScope::deserialize(std::string_view const serialOrLink) {
    Json tempDoc;
    tempDoc.deserialize(serialOrLink);
    static ScopedKeyView constexpr key("");
    std::string const fullKey = key.full(*this);
    doc().setSubDoc(fullKey, tempDoc);
}

//------------------------------------------
// Transform

bool JsonScope::transform(std::span<std::string_view const> const args){
    return JsonTransformer::instance().parseSingleTransformation(args, *this);
}

//------------------------------------------
// Access test

void JsonScope::assertAccess(ScopedKeyView const& key) const{
    (void)key.full(*this); // Just generate full key to test access
}

void JsonScope::assertAccess(ScopedKey const& key) const {
    assertAccess(key.view());
}

bool JsonScope::isDummy() const {
    return !scopePrefix.has_value();
}

} // namespace Nebulite::Data
