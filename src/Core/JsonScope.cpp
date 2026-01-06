#include "Nebulite.hpp"
#include "Core/JsonScope.hpp"
#include "Data/Document/JSON.hpp"
#include "DomainModule/Initializer.hpp"

// JsonScope methods
namespace Nebulite::Core {

//------------------------------------------
// Constructors for JsonScope

JsonScope::JsonScope(Data::JSON& doc, std::string const& prefix, std::string const& name)
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
            Data::JSON tmp;
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
JsonScope& JsonScope::shareScope(Data::ScopedKeyView const& key) const {
    return baseDocument->shareManagedScope(key.full(*this));
}

JsonScope& JsonScope::shareScope(std::string const& key) const {
    auto const scopedKey = Data::ScopedKeyView(getScopePrefix(), key);
    return baseDocument->shareManagedScope(scopedKey.full(*this));
}

} // namespace Nebulite::Core
