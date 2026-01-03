#include "Nebulite.hpp"
#include "Data/Document/JsonScope.hpp"
#include "Data/Document/JSON.hpp"

// scopedKey methods
namespace Nebulite::Data {

std::string JsonScopeBase::scopedKey::full(JsonScopeBase const& scope) const {
    // The scope that this JsonScopeBase is allowed to use
    std::string const& allowedScope = scope.scopePrefix;

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
                "' but was used in JsonScopeBase with prefix '" + allowedScope;
            throw std::invalid_argument(msg);
        }

        // Now we can safely use the given scope, as it lies within the allowed scope
        fullKey.reserve(given.size() + key.size());
        fullKey = given;
        fullKey.append(key);
        return fullKey;
    }
    else {
        fullKey.reserve(allowedScope.size() + key.size());
        fullKey = scope.scopePrefix;
        fullKey.append(key);
        return fullKey;
    }
}

} // namespace Nebulite::Data

// JsonScopeBase methods
namespace Nebulite::Data {
// Constructing a JsonScopeBase from a JSON document and a prefix
JsonScopeBase::JsonScopeBase(JSON& doc, std::string const& prefix)
    // create a non-owning shared_ptr to the provided JSON (no delete on destruction)
    : baseDocument(std::shared_ptr<JSON>(&doc, [](JSON*){})), scopePrefix(generatePrefix(prefix))
{}

// Constructing a JsonScopeBase from another JsonScopeBase and a sub-prefix
JsonScopeBase::JsonScopeBase(JsonScopeBase const& other, std::string const& prefix)
    : baseDocument(other.baseDocument),
      scopePrefix(scopedKey(generatePrefix(prefix)).full(other))  // Generate full scoped prefix based on the other JsonScopeBase and the new prefix
{}

// Default constructor, we create a self-owned empty JSON document
JsonScopeBase::JsonScopeBase()
    : baseDocument(std::make_shared<JSON>()), scopePrefix("")
{}

// --- Copy constructor
JsonScopeBase::JsonScopeBase(JsonScopeBase const& other)
    : baseDocument(other.baseDocument),
      scopePrefix(other.scopePrefix)
{}

// --- Move constructor
JsonScopeBase::JsonScopeBase(JsonScopeBase&& other) noexcept
    : baseDocument(std::move(other.baseDocument)),
      scopePrefix(std::move(other.scopePrefix))
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

JsonScopeBase::~JsonScopeBase() = default;

//------------------------------------------
// Sharing a scope

// Proper scope sharing with nested unscoped key generation
JsonScope& JsonScopeBase::shareScope(scopedKey const& key) const {
    return baseDocument->shareManagedScope(key.full(*this));
}

JsonScopeBase& JsonScopeBase::shareScopeBase(scopedKey const& key) const {
    return baseDocument->shareManagedScopeBase(key.full(*this));
}

//------------------------------------------
// Getter

[[nodiscard]] std::optional<RjDirectAccess::simpleValue> JsonScopeBase::getVariant(scopedKey const& key) const {
    return baseDocument->getVariant(key.full(*this));
}

[[nodiscard]] JSON JsonScopeBase::getSubDoc(scopedKey const& key) const {
    return baseDocument->getSubDoc(key.full(*this));
}

[[nodiscard]] double* JsonScopeBase::getStableDoublePointer(scopedKey const& key) const {
    return baseDocument->getStableDoublePointer(key.full(*this));
}

//------------------------------------------
// Setter

void JsonScopeBase::setVariant(scopedKey const& key, RjDirectAccess::simpleValue const& value) const {
    baseDocument->setVariant(key.full(*this), value);
}

void JsonScopeBase::setSubDoc(scopedKey const& key, JSON& subDoc) const {
    baseDocument->setSubDoc(key.full(*this), subDoc);
}

void JsonScopeBase::setSubDoc(scopedKey const& key, JsonScopeBase const& subDoc) const {
    // Slightly more complicated: If we wish to set the sub-document from another JsonScopeBase,
    // we need to extract the underlying JSON document from it in the correct scope.
    JSON subDocScope = subDoc.getSubDoc(scopedKey(""));
    baseDocument->setSubDoc(key.full(*this), subDocScope);
}

void JsonScopeBase::setEmptyArray(scopedKey const& key) const {
    baseDocument->setEmptyArray(key.full(*this));
}

//------------------------------------------
// Special sets for threadsafe maths operations

void JsonScopeBase::set_add(scopedKey const& key, double const& val) const {
    baseDocument->set_add(key.full(*this), val);
}

void JsonScopeBase::set_multiply(scopedKey const& key, double const& val) const {
    baseDocument->set_multiply(key.full(*this), val);
}

void JsonScopeBase::set_concat(scopedKey const& key, std::string const& valStr) const {
    baseDocument->set_concat(key.full(*this), valStr);
}

//------------------------------------------
// Locking

[[nodiscard]] std::scoped_lock<std::recursive_mutex> JsonScopeBase::lock() const {
    return baseDocument->lock();
}

//------------------------------------------
// Key Types, Sizes

[[nodiscard]] KeyType JsonScopeBase::memberType(scopedKey const& key) const {
    return baseDocument->memberType(key.full(*this));
}

[[nodiscard]] size_t JsonScopeBase::memberSize(scopedKey const& key) const {
    return baseDocument->memberSize(key.full(*this));
}

void JsonScopeBase::removeKey(scopedKey const& key) const {
    baseDocument->removeKey(key.full(*this));
}

//------------------------------------------
// Deserialize/Serialize

std::string JsonScopeBase::serialize(scopedKey const& key) const {
    return baseDocument->serialize(key.full(*this));
}

void JsonScopeBase::deserialize(std::string const& serialOrLink) {
    // No support for any tokens, just forward to baseDocument
    baseDocument->deserialize(serialOrLink);
}

} // namespace Nebulite::Data
