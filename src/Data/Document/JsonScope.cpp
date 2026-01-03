#include "Nebulite.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/JsonScope.hpp"
#include "DomainModule/Initializer.hpp"

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

/*

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

*/

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

// Proper scope sharing with nested unscoped key generation
JsonScope& JsonScope::shareScope(scopedKey const& key) const {
    return baseDocument->shareManagedScope(key.full(*this));
}

} // namespace Nebulite::Data
