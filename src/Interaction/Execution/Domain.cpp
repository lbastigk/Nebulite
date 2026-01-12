#include "Nebulite.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "../../../include/Core/JsonScope.hpp"

// Document Accessor
namespace Nebulite::Interaction::Execution {

DocumentAccessor::~DocumentAccessor() = default;


} // namespace Nebulite::Interaction::Execution

// Domain Base
namespace Nebulite::Interaction::Execution {

DomainBase& DomainBase::operator=(DomainBase const& other) {
    if (this == &other) return *this;
    if (&domainScope != &other.domainScope) {
        throw std::invalid_argument("DomainBase::operator=: cannot assign from object with different document reference");
    }

    domainName = other.domainName;
    funcTree = other.funcTree; // shared ownership
    if (funcTree) {
        // Rebind preParse to this object (avoid using other's bound callback)
        funcTree->setPreParse([this] { return preParse(); });
    }
    return *this;
}

DomainBase& DomainBase::operator=(DomainBase&& other) noexcept {
    if (this == &other) return *this;
    domainName = std::move(other.domainName);
    funcTree = std::move(other.funcTree);
    if (funcTree) {
        funcTree->setPreParse([this] { return preParse(); });
    }
    return *this;
}

DomainBase::~DomainBase() = default;

std::string const& DomainBase::scopePrefix() const {
    return domainScope.getScopePrefix();
}

Constants::Error DomainBase::parseStr(std::string const& str) {
    // NOTE: This may fail, as domainScope is from the domain itself, potentially larger than scopes from inner domains
    //       e.g. we may call parseStr from RenderObject, but the function exists in Texture domain with prefix "texture."
    //       In that case, we accidentally pass the RenderObject scope instead of the Texture scope...
    //       Is this okay? For now, we assume it is. Meaning we should not use callerScope in inner Domains with a prefix.
    //       CallerScope is, however, a nice addition for DomainModules that have no workspace like JsonScope SimpleData.
    //       This ensures that any calls only touch intended parts of the document.
    return funcTree->parseStr(str, *this, domainScope);
}

Data::MappedOrderedDoublePointers* DomainBase::getDocumentCacheMap() const {
    return domainScope.getOrderedCacheListMap();
}

std::scoped_lock<std::recursive_mutex> DomainBase::lockDocument() const {
    return domainScope.lock();
}

std::vector<std::string> DomainBase::stringToDeserializeTokens(std::string const& serialOrLinkWithCommands) {
    //------------------------------------------
    // Split the input into tokens
    std::vector<std::string> tokens;
    if (Data::JSON::isJsonOrJsonc(serialOrLinkWithCommands)) {
        // Direct JSON string, no splitting
        tokens.push_back(serialOrLinkWithCommands);
    } else {
        // Split based on transformations, indicated by '|'
        tokens = Utility::StringHandler::split(serialOrLinkWithCommands, '|');
    }
    return tokens;
}

void DomainBase::baseDeserialization(std::string const& serialOrLinkWithCommands) {
    //------------------------------------------
    // Split the input into tokens
    auto tokens = stringToDeserializeTokens(serialOrLinkWithCommands);
    if (tokens.empty()) {
        return;
    }

    //------------------------------------------
    // Load the JSON file

    // Pass only the serial/link part to deserialize
    // Argument parsing happens at the higher level
    std::string const serialOrLink = tokens[0];
    domainScope.deserialize(serialOrLink);

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

} // namespace Nebulite::Interaction::Execution
