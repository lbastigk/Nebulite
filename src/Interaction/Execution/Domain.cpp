//------------------------------------------
// Includes

// Standard library
#include <atomic>
#include <cstddef>
#include <cstdint> // NOLINT
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/Execution/DomainTree.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Module/Domain/Common/Ruleset.hpp"
#include "Nebulite/Module/Domain/Initializer.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Execution {

ScopeOwnershipManager::~ScopeOwnershipManager() = default;

ScopeOwnershipManager::ScopeOwnershipManager(ScopeOwnership const ownership) {
    if (ownership == ScopeOwnership::Owned) {
        domainScopeOwned.emplace();
    }
}

DocumentAccessor::DocumentAccessor(Data::JsonScope& d) : ScopeOwnershipManager(ScopeOwnership::Borrowed), domainScope(d) {}

DocumentAccessor::DocumentAccessor() : ScopeOwnershipManager(ScopeOwnership::Owned), domainScope(domainScopeOwned.value()) {
    // Note: This creates a new JsonScope that is owned by this DocumentAccessor.
    // It will be automatically cleaned up when the DocumentAccessor is destroyed.
}

DocumentAccessor::~DocumentAccessor() = default;

Domain::Domain(std::string const& name, Data::JsonScope& documentReference, Utility::IO::Capture& parentCapture)
    : DocumentAccessor(documentReference)
    , domainName(name)
    , capture(&parentCapture)
    , cost(domainScope){
    // FuncTree initialization
    funcTree = std::make_shared<DomainTree>(
        name,
        Constants::Event::Success,
        Constants::Event::Warning,
        capture
    );
    funcTree->setPreParse([this] { return preParse(); });

    // Initialize modules
    Module::Domain::Initializer::initCommon(this);
}

Domain::Domain(std::string const& name, Utility::IO::Capture& parentCapture)
    : domainName(name)
    , capture(&parentCapture)
    , cost(domainScope) {
    // FuncTree initialization
    funcTree = std::make_shared<DomainTree>(
        name,
        Constants::Event::Success,
        Constants::Event::Warning,
        capture
    );
    funcTree->setPreParse([this] { return preParse(); });

    // Initialize modules
    Module::Domain::Initializer::initCommon(this);
}

Domain::Domain(std::string const& name, Data::JsonScope& documentReference)
    : DocumentAccessor(documentReference)
    , domainName(name)
    , capture(nullptr)
    , cost(domainScope) {
    // FuncTree initialization
    funcTree = std::make_shared<DomainTree>(
        name,
        Constants::Event::Success,
        Constants::Event::Warning,
        capture
    );
    funcTree->setPreParse([this] { return preParse(); });

    // Initialize modules
    Module::Domain::Initializer::initCommon(this);
}

Domain::Domain(std::string const& name)
    : domainName(name)
    , capture(nullptr)
    , cost(domainScope){
    // FuncTree initialization
    funcTree = std::make_shared<DomainTree>(
        name,
        Constants::Event::Success,
        Constants::Event::Warning,
        capture
    );
    funcTree->setPreParse([this] { return preParse(); });

    // Initialize modules
    Module::Domain::Initializer::initCommon(this);
}

Domain::~Domain() {
    modules.clear();
}

std::string const& Domain::scopePrefix() const {
    return domainScope.getScopePrefix();
}

Constants::Event Domain::parseStr(std::string_view const cmd, Context& ctx, ContextScope& ctxScope) const {
    return funcTree->parseStr(cmd, ctx, ctxScope);
}

Constants::Event Domain::parseWithPrefix(std::vector<std::string_view>& existingArgs, std::string_view const cmd, Context& ctx, ContextScope& ctxScope) const {
    return funcTree->parseWithPrefix(existingArgs, cmd, ctx, ctxScope);
}

Constants::Event Domain::parse(std::span<std::string_view> const args, Context& ctx, ContextScope& ctxScope) const {
    return funcTree->parse(args, ctx, ctxScope);
}

Constants::Event Domain::parse(std::vector<std::string_view> const& args, Context& ctx, ContextScope& ctxScope) const {
    return funcTree->parse(args, ctx, ctxScope);
}

Constants::Event Domain::parse(std::vector<std::string> const& args, Context& ctx, ContextScope& ctxScope) const {
    return funcTree->parse(args, ctx, ctxScope);
}

double** Domain::ensureOrderedCacheList(std::uint64_t const uniqueId, std::vector<Data::ScopedKeyView> const& keys) const {
    return domainScope.ensureOrderedCacheList(uniqueId, keys);
}

std::unique_lock<std::recursive_mutex> Domain::lockDocument() const {
    return domainScope.lock();
}

std::vector<std::string> Domain::stringToDeserializeTokens(std::string_view const serialOrLinkWithCommands) {
    //------------------------------------------
    // Split the input into tokens
    std::vector<std::string> tokens;
    if (Data::JSON::isJsonOrJsonc(serialOrLinkWithCommands)) {
        // Direct JSON string, no splitting
        tokens.emplace_back(serialOrLinkWithCommands);
    } else {
        // Split based on transformations, indicated by '|'
        for (auto const& token : Utility::StringHandler::split(serialOrLinkWithCommands, '|')) {
            tokens.emplace_back(token);
        }
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
        auto parts = Utility::StringHandler::splitOnSameDepthOf(serialOrLinkWithCommands, Utility::StringHandler::Delimiter::brace);

        // First part is the variable with transformations
        auto const& variableWithTransformations = parts[0];

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
        auto const& serialOrLink = tokens[0];
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
            auto keyAndValue = std::string(token);
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

void Domain::parseTaskQueues(bool const recover){
    Global::instance().notifyEvent(tasks.parse(*this, domainScope, recover));
}

// Cost

Domain::Cost::Cost(Data::JsonScope const& scope) {
    Module::Domain::Common::Ruleset::Key const keys(scope);
    local = scope.getStableDoublePointer(keys.costLocal);
    global = scope.getStableDoublePointer(keys.costGlobal);
}

uint64_t Domain::estimateComputationalCost(bool const onlyInternal) const {
    // TODO: Consider cost of inner domains, make function virtual and provide ownCost function for cost per domain
    //       and then this function returns ownCost + cost of inner domains.
    if (onlyInternal) {
        return static_cast<uint64_t>(*cost.local);
    }
    return static_cast<uint64_t>(*cost.global) + static_cast<uint64_t>(*cost.local);
}

// Identifier

std::size_t Domain::Identifier::idGenerator() {
    static std::atomic<std::size_t> idCounter{0};
    idCounter.fetch_add(1, std::memory_order_relaxed);
     std::size_t const id = idCounter.load(std::memory_order_relaxed) - 1; // Get the current value before incrementing
    return id;
}

std::size_t Domain::Identifier::splitMix64(std::size_t x) {
    x += 0x9e3779b97f4a7c15;
    x = (x ^ x >> 30) * 0xbf58476d1ce4e5b9;
    x = (x ^ x >> 27) * 0x94d049bb133111eb;
    x = x ^ x >> 31;
    return x;
}

void Domain::Identifier::init() {
    id = idGenerator();
    idHashed = splitMix64(id);
}

std::size_t Domain::Identifier::getId() {
    std::call_once(initialized, &Identifier::init, this);
    return id;
}

std::size_t Domain::Identifier::getIdHashed() {
    std::call_once(initialized, &Identifier::init, this);
    return idHashed;
}

} // namespace Nebulite::Interaction::Execution
