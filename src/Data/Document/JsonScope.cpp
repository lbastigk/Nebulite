#include "Nebulite.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/JsonScope.hpp"
#include "DomainModule/Initializer.hpp"

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
// Deserialize/Serialize

void JsonScopeBase::deserialize(std::string const& serialOrLink) {
    // No support for any tokens, just forward to baseDocument
    baseDocument->deserialize(serialOrLink);
}
} // namespace Nebulite::Data

// JsonScope methods
namespace Nebulite::Data {

//------------------------------------------
// Constructors for JsonScope



JsonScope::JsonScope(JSON& doc, std::string const& prefix, std::string const& name)
    : Domain(name, *this, *this),
      JsonScopeBase(doc, prefix)
{
    DomainModule::Initializer::initJsonScope(this);
}

JsonScope::JsonScope(JsonScope const& other, std::string const& prefix, std::string const& name)
    : Domain(name, *this, *this),
      JsonScopeBase(other, prefix)
{
    DomainModule::Initializer::initJsonScope(this);
}

JsonScope::JsonScope(std::string const& name)
    : Domain(name, *this, *this)
{
    DomainModule::Initializer::initJsonScope(this);
}

//------------------------------------------
// Special member functions for JsonScope

// --- Copy constructor
JsonScope::JsonScope(JsonScope const& other)
    : Domain(this->getName(), *this, *this),
      JsonScopeBase(other)
{
    // If you need to re-register modules / reinit state after copy, do it here:
    reinitModules();
}

// --- Move constructor
JsonScope::JsonScope(JsonScope&& other) noexcept
    : Domain(this->getName(), *this, *this),
      JsonScopeBase(std::move(other))
{}

// --- Copy assignment (copy-and-swap)
JsonScope& JsonScope::operator=(JsonScope const& other) {
    if (this == &other) return *this;
    JsonScope tmp(other);
    std::swap(*this, tmp);
    reinitModules();
    return *this;
}

// --- Move assignment (copy-and-swap with moved temporary)
JsonScope& JsonScope::operator=(JsonScope&& other) noexcept {
    if (this == &other) return *this;
    JsonScope tmp(std::move(other));
    std::swap(*this, tmp);
    return *this;
}

void JsonScope::deserialize(std::string const& serialOrLink) {
    //------------------------------------------
    // Split the input into tokens
    std::vector<std::string> tokens;
    if (JSON::isJsonOrJsonc(serialOrLink)) {
        // Direct JSON string, no splitting
        tokens.push_back(serialOrLink);
    } else {
        // Split based on transformations, indicated by '|'
        tokens = Utility::StringHandler::split(serialOrLink, '|');
    }

    //------------------------------------------
    // Validity check
    if (tokens.empty()) {
        // Error: No file path given
        return; // or handle error properly
    }

    //------------------------------------------
    // Load the JSON file
    if (getScopePrefix().empty()) {
        // Edge case: no scope prefix, we can deserialize directly
        baseDocument->deserialize(serialOrLink);
    }
    else {
        // Deserialize into a temporary JSON, then set as sub-document
        JSON tmp;
        tmp.deserialize(serialOrLink);
        auto scopePrefixWithoutDot = getScopePrefix();
        if (!scopePrefixWithoutDot.empty() && scopePrefixWithoutDot.ends_with(".")) {
            scopePrefixWithoutDot = scopePrefixWithoutDot.substr(0, scopePrefixWithoutDot.size() - 1);
        }
        baseDocument->setSubDoc(scopePrefixWithoutDot, tmp);
    }

    //------------------------------------------
    // Re-initialize modules first
    reinitModules();

    //------------------------------------------
    // Now apply modifications
    tokens.erase(tokens.begin()); // Remove the first token (path or serialized JSON)
    for (auto const& token : tokens) {
        if (token.empty())
            continue; // Skip empty tokens

        // Legacy: Handle key=value pairs
        if (auto const pos = token.find('='); pos != std::string::npos) {
            // Handle transformation (key=value)
            std::string keyAndValue = token;
            if (pos != std::string::npos)
                keyAndValue[pos] = ' ';

            // New implementation through functioncall
            if (std::string const callStr = std::string(__FUNCTION__) + " set " + keyAndValue; parseStr(callStr) != Constants::ErrorTable::NONE()) {
                Nebulite::cerr() << "Failed to apply deserialize transformation: " << callStr << Nebulite::endl;
            }
        } else {
            // Forward to FunctionTree for resolution
            if (std::string const callStr = std::string(__FUNCTION__) + " " + token; parseStr(callStr) != Constants::ErrorTable::NONE()) {
                Nebulite::cerr() << "Failed to apply deserialize transformation: " << callStr << Nebulite::endl;
            }
        }
    }
}

} // namespace Nebulite::Data
