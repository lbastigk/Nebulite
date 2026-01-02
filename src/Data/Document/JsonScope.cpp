#include "Nebulite.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/JsonScope.hpp"
#include "DomainModule/Initializer.hpp"

namespace Nebulite::Data {

// Constructing a JsonScope from a JSON document and a prefix
JsonScope::JsonScope(JSON& doc, std::string const& prefix, std::string const& name)
    : Domain(name, *this, *this),
    // create a non-owning shared_ptr to the provided JSON (no delete on destruction)
    baseDocument(std::shared_ptr<JSON>(&doc, [](JSON*){})), scopePrefix(generatePrefix(prefix)) {
    DomainModule::Initializer::initJsonScope(this);
}

// Constructing a JsonScope from another JsonScope and a sub-prefix
JsonScope::JsonScope(JsonScope const& other, std::string const& prefix, std::string const& name)
    : Domain(name, *this, *this),
      baseDocument(other.baseDocument),
      scopePrefix(scopedKey(generatePrefix(prefix)).full(other))  // Generate full scoped prefix based on the other JsonScope and the new prefix
{
    DomainModule::Initializer::initJsonScope(this);
}

// Default constructor, we create a self-owned empty JSON document
JsonScope::JsonScope(std::string const& name)
    : Domain(name, *this, *this),
      baseDocument(std::make_shared<JSON>()), scopePrefix("") {
    DomainModule::Initializer::initJsonScope(this);
}

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

//------------------------------------------
// Deserialize/Serialize

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
    if (scopePrefix.empty()) {
        // Edge case: no scope prefix, we can deserialize directly
        baseDocument->deserialize(serialOrLink);
    }
    else {
        // Deserialize into a temporary JSON, then set as sub-document
        JSON tmp;
        tmp.deserialize(serialOrLink);
        auto scopePrefixWithoutDot = scopePrefix;
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
