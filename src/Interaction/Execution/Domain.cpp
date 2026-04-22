#include "Nebulite.hpp"
#include "Data/Document/JsonScope.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Module/Domain/Initializer.hpp"
#include "Interaction/Context.hpp"

#include <vector>

// Document Accessor
namespace Nebulite::Interaction::Execution {

ScopeOwnershipManager::~ScopeOwnershipManager() = default;

ScopeOwnershipManager::ScopeOwnershipManager(ScopeOwnership const& ownership) {
    if (ownership == ScopeOwnership::Owned) {
        _domainScopeOwned = std::make_unique<Data::JsonScope>();
    }
}

DocumentAccessor::DocumentAccessor(Data::JsonScope& d) : ScopeOwnershipManager(ScopeOwnership::Borrowed), domainScope(d) {}

DocumentAccessor::DocumentAccessor() : ScopeOwnershipManager(ScopeOwnership::Owned), domainScope(*_domainScopeOwned) {
    // Note: This creates a new JsonScope that is owned by this DocumentAccessor.
    // It will be automatically cleaned up when the DocumentAccessor is destroyed.
}

DocumentAccessor::~DocumentAccessor() = default;

} // namespace Nebulite::Interaction::Execution

namespace Nebulite::Interaction::Execution {

// NOLINTNEXTLINE
Domain::Domain(std::string const& name, Data::JsonScope& documentReference, Utility::IO::Capture& parentCapture) : DocumentAccessor(documentReference), domainName(name), capture(&parentCapture) {
    // FuncTree initialization
    funcTree = std::make_shared<DomainTree>(
        name,
        Constants::Event::Success,
        Constants::Event::Warning,
        capture
    );
    funcTree->setPreParse([this] { return preParse(); });

    // Initialize modules
    Nebulite::DomainModule::Initializer::initCommon(this);
}

Domain::Domain(std::string const& name, Utility::IO::Capture& parentCapture) : domainName(name), capture(&parentCapture) {
    // FuncTree initialization
    funcTree = std::make_shared<DomainTree>(
        name,
        Constants::Event::Success,
        Constants::Event::Warning,
        capture
    );
    funcTree->setPreParse([this] { return preParse(); });

    // Initialize modules
    Nebulite::DomainModule::Initializer::initCommon(this);
}

Domain::Domain(std::string const& name, Data::JsonScope& documentReference) : DocumentAccessor(documentReference), domainName(name), capture(nullptr) {
    // FuncTree initialization
    funcTree = std::make_shared<DomainTree>(
        name,
        Constants::Event::Success,
        Constants::Event::Warning,
        capture
    );
    funcTree->setPreParse([this] { return preParse(); });

    // Initialize modules
    Nebulite::DomainModule::Initializer::initCommon(this);
}

Domain::Domain(std::string const& name) : domainName(name), capture(nullptr) {
    // FuncTree initialization
    funcTree = std::make_shared<DomainTree>(
        name,
        Constants::Event::Success,
        Constants::Event::Warning,
        capture
    );
    funcTree->setPreParse([this] { return preParse(); });

    // Initialize modules
    Nebulite::DomainModule::Initializer::initCommon(this);
}

Domain::~Domain() = default;

std::string const& Domain::scopePrefix() const {
    return domainScope.getScopePrefix();
}

Constants::Event Domain::parseStr(std::string const& str, Context& ctx, ContextScope& ctxScope) const {
    // TODO: Later on we need pass ctx to parseStr
    return funcTree->parseStr(str, ctx, ctxScope);
}

double** Domain::ensureOrderedCacheList(uint64_t const& uniqueId, std::vector<Data::ScopedKeyView> const& keys) const {
    return domainScope.ensureOrderedCacheList(uniqueId, keys);
}

std::unique_lock<std::recursive_mutex> Domain::lockDocument() const {
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
        ContextScope const ctxBase{domainScope, domainScope, Global::instance().domainScope};

        // Parse into expression
        Data::JSON const result = Logic::Expression::evalAsJson(variableWithTransformations, ctxBase);

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
        // At this point, all context and all contextScope is the domain itself.
        Context ctx{*this, *this, Global::instance()};
        if (ContextScope ctxScope{domainScope, domainScope, Global::instance().domainScope}; parseStr(callStr, ctx, ctxScope) != Constants::Event::Success) {
            capture.error.println("Failed to apply deserialize transformation: ", callStr);
        }
    }
}

void Domain::updateModules() const {
    for (auto const& module : modules) {
        Global::instance().notifyEvent(module->update());
    }
}

} // namespace Nebulite::Interaction::Execution
