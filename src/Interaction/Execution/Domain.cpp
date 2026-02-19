#include "Nebulite.hpp"
#include "Core/JsonScope.hpp"
#include "Data/Document/JsonScopeBase.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Context.hpp"

#include <vector>

// Document Accessor
namespace Nebulite::Interaction::Execution {

DocumentAccessor::DocumentAccessor(Core::JsonScope& d) : domainScope(d) {}

DocumentAccessor::~DocumentAccessor() = default;

} // namespace Nebulite::Interaction::Execution

// Domain Base
namespace Nebulite::Interaction::Execution {

Domain& Domain::operator=(Domain const& other) {
    if (this == &other) return *this;
    if (&domainScope != &other.domainScope) {
        throw std::invalid_argument("Domain::operator=: cannot assign from object with different document reference");
    }

    domainName = other.domainName;
    funcTree = other.funcTree; // shared ownership
    if (funcTree) {
        // Rebind preParse to this object (avoid using other's bound callback)
        funcTree->setPreParse([this] { return preParse(); });
    }
    return *this;
}

Domain& Domain::operator=(Domain&& other) noexcept {
    if (this == &other) return *this;
    domainName = std::move(other.domainName);
    funcTree = std::move(other.funcTree);
    if (funcTree) {
        funcTree->setPreParse([this] { return preParse(); });
    }
    return *this;
}

Domain::~Domain() = default;

std::string const& Domain::scopePrefix() const {
    return domainScope.getScopePrefix();
}

Constants::Error Domain::parseStr(std::string const& str) {
    // NOTE: This may fail, as domainScope is from the domain itself, potentially larger than scopes from inner domains
    //       e.g. we may call parseStr from RenderObject, but the function exists in Texture domain with prefix "texture."
    //       In that case, we accidentally pass the RenderObject scope instead of the Texture scope...
    //       Is this okay? For now, we assume it is. Meaning we should not use callerScope in inner Domains with a prefix.
    //       CallerScope is, however, a nice addition for DomainModules that have no workspace like JsonScope SimpleData.
    //       This ensures that any calls only touch intended parts of the document.
    return funcTree->parseStr(str, *this, domainScope);
}

Data::MappedOrderedDoublePointers* Domain::getDocumentCacheMap() const {
    return domainScope.getOrderedCacheListMap();
}

Data::odpvec* Domain::ensureOrderedCacheList(uint64_t const& uniqueId, std::vector<Data::ScopedKeyView> const& keys) const {
    return domainScope.ensureOrderedCacheListMinimalLock(uniqueId, keys);
}

std::scoped_lock<std::recursive_mutex> Domain::lockDocument() const {
    return domainScope.lock();
}

std::vector<std::string> Domain::stringToDeserializeTokens(std::string const& serialOrLinkWithCommands) {
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

void Domain::baseDeserialization(std::string const& serialOrLinkWithCommands) {
    std::vector<std::string> tokens;

    //------------------------------------------
    // Check if the input is of type {variable|t1|t2|...}|c1|c2|...

    // Meaning the first char is '{', but the string is not a valid JSON object
    if (!serialOrLinkWithCommands.empty() && serialOrLinkWithCommands.front() == '{' && !Data::JSON::isJsonOrJsonc(serialOrLinkWithCommands)) {
        // Split on same depth of '{' and '}' to isolate the variable part
        auto parts = Utility::StringHandler::splitOnSameDepth(serialOrLinkWithCommands, '{');

        // First part is the variable with transformations
        std::string const& variableWithTransformations = parts[0];

        // Setup context for parsing
        Context const ctx{domainScope, domainScope, Global::instance().domainScope};

        // Parse into expression
        Data::JSON const result = Logic::Expression::evalAsJson(variableWithTransformations, ctx);

        // Deserialize the resulting JSON into the domain scope
        domainScope.deserialize(result.serialize());

        // Recombine the parts after the first part into commands
        if (!parts.empty()) {
            parts.erase(parts.begin()); // Remove the first part
        }

        // Split the rest into tokens based on '|'
        tokens = stringToDeserializeTokens(std::accumulate(parts.begin(), parts.end(), std::string{}));
    }
    else {
        //------------------------------------------
        // Split the input into tokens
        tokens = stringToDeserializeTokens(serialOrLinkWithCommands);
        if (tokens.empty()) {
            return;
        }

        //------------------------------------------
        // Load the JSON file

        // Pass only the serial/link part to deserialize
        // Argument parsing happens at the higher level
        std::string const serialOrLink = tokens[0];
        domainScope.deserialize(serialOrLink);
        tokens.erase(tokens.begin()); // Remove the first token (path or serialized JSON)
    }

    //------------------------------------------
    // Domain-Serialization-Piping
    for (auto const& token : tokens) {
        if (token.empty())
            continue; // Skip empty tokens

        // Legacy: Handle key=value pairs
        std::string callStr;
        if (auto const pos = token.find('='); pos != std::string::npos) {
            // Handle transformation (key=value)
            std::string keyAndValue = token;
            if (pos != std::string::npos)
                keyAndValue[pos] = ' ';

            // New implementation through functioncall
            callStr = std::string(__FUNCTION__) + " set " + keyAndValue;
        } else {
            callStr = std::string(__FUNCTION__) + " " + token;
        }
        // Forward to FunctionTree for resolution
        if (parseStr(callStr) != Constants::ErrorTable::NONE()) {
            Error::println("Failed to apply deserialize transformation: ", callStr);
        }
    }
}

} // namespace Nebulite::Interaction::Execution
