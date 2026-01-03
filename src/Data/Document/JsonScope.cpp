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

void JsonScope::deserialize(std::string const& serialOrLinkWithCommands) {
    // Two options: If serialOrLinkWithCommands has commands, we forward it to baseDeserialization
    // If not, we reached the lowest level and just deserialize normally

    //------------------------------------------
    // Split the input into tokens
    auto const tokens = stringToDeserializeTokens(serialOrLinkWithCommands);
    if (tokens.empty()) {
        return;
    }

    //------------------------------------------
    // Load the JSON file
    std::string const serialOrLink = tokens[0];
    if (tokens.size() == 1) {
        // No commands left, just deserialize normally
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
    }
    else {
        // Commands present, forward to baseDeserialization
        baseDeserialization(serialOrLinkWithCommands);
    }

    // Re-initialize json scope modules after deserialization
    reinitModules();
}

// Proper scope sharing with nested unscoped key generation
JsonScope& JsonScope::shareScope(ScopedKey const& key) const {
    return baseDocument->shareManagedScope(key.full(*this));
}

} // namespace Nebulite::Data
