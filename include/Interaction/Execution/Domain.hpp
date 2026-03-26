/**
 * @file Domain.hpp
 * @brief This file defines the Domain class, which serves as a base class for creating a Nebulite domain.
 *        The Domain class is split into a templated Domain class and a non-templated Domain class.
 *        The non-templated Domain class holds all common functionality for domains that do not require template parameters.
 */

#ifndef NEBULITE_INTERACTION_EXECUTION_DOMAIN_HPP
#define NEBULITE_INTERACTION_EXECUTION_DOMAIN_HPP

//------------------------------------------
// Macro to define a new Nebulite Domain class

// NOLINTNEXTLINE
#define NEBULITE_DOMAIN(DomainName) \
    class DomainName final : public Nebulite::Interaction::Execution::Domain

//------------------------------------------
// Includes

// Standard library
#include <string_view>

// Nebulite
#include "Constants/StandardCapture.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Interaction/Execution/FuncTree.hpp"

//------------------------------------------
// Forward declarations: Domains

namespace Nebulite::Core {
class Environment;
class GlobalSpace;
class Renderer;
class RenderObject;
class Texture;
} // namespace Nebulite::Core

//------------------------------------------
// Forward declarations: Other classes

namespace Nebulite::Data {
class MappedOrderedCacheList;
class TaskQueue;
class ScopedKeyView;
class JsonScope;
} // namespace Nebulite::Data

namespace Nebulite::DomainModule {
class Initializer;
} // namespace Nebulite::DomainModule

namespace Nebulite::Interaction {
class Context; // Requires access to demote to ContextScope
} // namespace Nebulite::Interaction

namespace Nebulite::Interaction::Logic {
class Assignment;   // Requires access to set target documents
class Expression;   // Requires access to get unscoped values from global scope
} // namespace Nebulite::Interaction::Logic

namespace Nebulite::Interaction::Rules {
class Ruleset;
class JsonRuleset;
class StaticRuleset;
} // namespace Nebulite::Interaction::Rules

namespace Nebulite::Interaction::Rules::Construction {
class RulesetCompiler;
} // namespace Nebulite::Interaction::Rules::Construction

//------------------------------------------
// Document Accessor

namespace Nebulite::Interaction::Execution {

// Helps not expose the domainScopeOwned to friend classes of DocumentAccessor
class ScopeOwnershipManager {
    friend class DocumentAccessor;
    // Only used if the DocumentAccessor owns the scope (i.e., when constructed with the default constructor)
    std::unique_ptr<Data::JsonScope> _domainScopeOwned;

public:
    enum class ScopeOwnership {
        Owned, // Will create and own a new JsonScope (default constructor)
        Borrowed // Will be left empty
    };

    virtual ~ScopeOwnershipManager();
    explicit ScopeOwnershipManager(ScopeOwnership const& ownership = ScopeOwnership::Borrowed);
};

/**
 * @brief DocumentAccessor provides controlled access to a domain's JSON document.
 * @details This class is designed to be a friend of various domain-related classes,
 *          allowing them to access and manipulate the domain's JSON document safely.
 *          This ensures per-class access control while maintaining encapsulation.
 *          For example, DomainModules should not have direct access to the domain's document,
 *          due to the encapsulation they provide. Instead, we pass a scope to them.
 */
class DocumentAccessor : ScopeOwnershipManager {
public:
    explicit DocumentAccessor(Data::JsonScope& d);

    explicit DocumentAccessor(); // Creates a new JsonScope owned by this DocumentAccessor

    ~DocumentAccessor() override;

    friend class Domain;

    // Allow other Domains to access documents from each other.
    friend class Core::Environment;
    friend class Core::GlobalSpace;
    friend class Core::Renderer;
    friend class Core::RenderObject;
    friend class Core::Texture;

    // Allow TaskQueue access to set caller scope
    friend class Data::TaskQueue;

    // Allow Context to demote to ContextScope
    // NOLINTNEXTLINE
    friend class Interaction::Context; // Clang says the Interaction namespace is not needed, but without it, the Context demotion fails.

    // Assignments and Expressions need access to set/get document values
    friend class Logic::Assignment;
    friend class Logic::Expression;

    // Some parts of the JSON ruleset need access to the domain document for parsing and execution
    friend class Rules::JsonRuleset;
    friend class Rules::Construction::RulesetCompiler;

    // Initializer needs access to share scopes
    friend class Nebulite::DomainModule::Initializer;

private:
    /**
     * @brief Each domain uses a JSON document to store its data.
     */
    Data::JsonScope& domainScope;
};
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
// Domain Base

namespace Nebulite::Interaction::Execution {

/**
 * @brief Concept to check if a DomainModuleType has a static member `Key::scope` that defines the scope for the module.
 * @details This concept is used to ensure that a DomainModuleType has the necessary static member to be initialized with the correct scope in the `initModule` function of the Domain class.
 */
template <typename T>
concept HasKeyGroup = requires {
    typename T::Key;           // nested Key type exists
    { T::Key::getScope() };    // Key::getScope() exists (now it's likely that Key inherits from KeyGroup.
};
// Checking directly if Key inherits from KeyGroup would be more complex, so we ignore it for now...

/**
 * @class Domain
 * @brief The Domain class serves as a base class for creating a Nebulite domain.
 * @details Each domain has the following features:
 *          - Setting and getting values in its internal JSON document.
 *          - Returning a pointer to its internal JSON document.
 *          - Parsing strings into Nebulite commands.
 *          - Binding additional features via DomainModules.
 *          - Updating the domain through its DomainModules.
 */
class Domain : public DocumentAccessor {
    /**
     * @brief The name of the domain.
     */
    std::string domainName;

    /**
     * @brief Parsing interface for domain-specific commands.
     * @details We use a pointer here so we can
     *          easily create the object with an inherited FuncTree inside the constructor.
     *          The Tree is then shared with the DomainModules for modification.
     *          - Constants::Event: Return type of the funcTree
     *          - Domain&: Provides access to the caller domain
     *          - Data::JsonScope&: provides access to the callers scope
     */
    std::shared_ptr<FuncTree<Constants::Event, Domain&, Data::JsonScope&>> funcTree;

    /**
     * @brief Stores all available modules
     */
    std::vector<std::unique_ptr<DomainModuleBase>> modules;

    /**
     * @brief Generates the unique id of this domain
     * @return The id generated
     */
    static size_t idGenerator() {
        static std::atomic<size_t> idCounter{0};
        idCounter.fetch_add(1, std::memory_order_relaxed);
        size_t const id = idCounter.load(std::memory_order_relaxed) - 1; // Get the current value before incrementing
        return id;
    }

    /**
     * @brief Generates a hashed version of the domain id.
     * @details This is done for proper distribution of the id to workers.
     *          If every RenderObject has N many subdomains, the RenderObject (R) id will likely be
     *          N*R+M, meaning the modulo is always the same. That's not optimal for proper worker distribution.
     * @param x The id input
     * @return The hashed id
     */
    static size_t splitMix64(size_t x) {
        x += 0x9e3779b97f4a7c15;
        x = (x ^ x >> 30) * 0xbf58476d1ce4e5b9;
        x = (x ^ x >> 27) * 0x94d049bb133111eb;
        x = x ^ x >> 31;
        return x;
    }

    // Unique ID for the domain, used for ordered cache lists and other purposes
    size_t id = idGenerator();

    // Hashed ID for better distribution
    size_t idHashed = splitMix64(id);

public:
    Domain(std::string const& name, Data::JsonScope& documentReference, Utility::Capture& parentCapture);

    explicit Domain(std::string const& name, Utility::Capture& parentCapture);

    Domain(std::string const& name, Data::JsonScope& documentReference);

    explicit Domain(std::string const& name);

    ~Domain() override ;

    //------------------------------------------
    // Disallow copying and moving

    Domain(Domain const&) = delete;
    Domain& operator=(Domain const& other) = delete;
    Domain(Domain&&) = delete;
    Domain& operator=(Domain&& other) = delete;

    //------------------------------------------
    // Set new name

    void setName(std::string const& newName) {
        domainName = newName;
    }

    //------------------------------------------
    // Get Document prefix

    [[nodiscard]] std::string const& scopePrefix() const ;

    //------------------------------------------
    // Binding, initializing and inheriting

    /**
     * @brief Binds a variable to the FuncTree.
     */
    void bindVariable(bool* varPtr, std::string_view const& name, std::string_view const& helpDescription) const {
        funcTree->bindVariable(varPtr, name, helpDescription);
    }

    /**
     * @brief Binds all functions from an inherited FuncTree to the main FuncTree for parsing.
     */
    void inherit(Domain const* toInheritFrom) const {
        if (toInheritFrom != nullptr) {
            funcTree->inherit(toInheritFrom->funcTree);
        }
    }

    [[nodiscard]] size_t const& getId() const {
        return id;
    }

    [[nodiscard]] size_t const& getIdHashed() const {
        return idHashed;
    }

    //------------------------------------------
    // Updating

    /**
     * @brief Updates the domain.
     *        On overwriting, make sure to update all subdomains and DomainModules as well.
     */
    [[nodiscard]] virtual Constants::Event update() { return Constants::Event::Success; }

    //------------------------------------------
    // Module Initialization and Updating

    /**
     * @brief Creates a DomainModule of the specified type with proper linkage, deriving the scope from the module's static scope member.
     * @tparam DomainType The type of the domain to link the module to
     * @tparam DomainModuleType The type of module to initialize, must have a static member `Key::scope` that defines the scope for the module, else a dummy scope will be shared (no workspace)
     * @param moduleName The name of the module
     * @param settings The settings JsonScope for the module
     * @param domainReference Reference to the domain to link the module to
     * @param funcTree Shared FuncTree for the module to modify, should be the same FuncTree as the Domain's main FuncTree
     * @return A unique pointer to the created DomainModule of the specified type
     */
    template <typename DomainType, typename DomainModuleType>
    static std::unique_ptr<DomainModuleType> createModule(std::string const& moduleName, Data::JsonScope const& settings, DomainType& domainReference, std::shared_ptr<FuncTree<Constants::Event, Domain&, Data::JsonScope&>> funcTree) {
        // Determine the key from root level
        if constexpr (HasKeyGroup<DomainModuleType>) {
            if (DomainModuleType::Key::hasScope()) {
                static auto const scopeKey = Data::ScopedKey("", DomainModuleType::Key::getScope());

                // Share the scope based on the module's defined scope
                auto& scope = domainReference.domainScope.shareScope(scopeKey); // Sharing a scope based on the module's defined scope
                auto DomainModule = std::make_unique<DomainModuleType>(moduleName, domainReference, funcTree, scope, settings);
                DomainModule->reinit();
                return DomainModule;
            }
            // No scope defined, share a dummy scope (no workspace)
            auto& scope = domainReference.domainScope.shareDummyScopeBase();
            auto DomainModule = std::make_unique<DomainModuleType>(moduleName, domainReference, funcTree, scope, settings);
            DomainModule->reinit();
            return DomainModule;
        }
        else {
            // No scope defined, share a dummy scope (no workspace)
            auto& scope = domainReference.domainScope.shareDummyScopeBase();
            auto DomainModule = std::make_unique<DomainModuleType>(moduleName, domainReference, funcTree, scope, settings);
            DomainModule->reinit();
            return DomainModule;
        }
    }

    /**
     * @brief Initializes DomainModules based on the template parameter with proper linkage, deriving the scope from the module's static scope member.
     * @details If no struct Key inheriting from KeyGroup is defined in the DomainModuleType, a dummy scope will be shared (no workspace)
     * @tparam DomainType The type of the domain to link the module to, used for sharing the correct scope
     * @tparam DomainModuleType The type of module to initialize, must have a static member `Key::scope` that defines the scope for the module
     * @param moduleName The name of the module
     * @param settings The settings JsonScope for the module
     * @param domainReference Reference to the domain to link the module to
     * @todo Later on we should check if the module already exists and reject initialization if it does.
     *       Either always looping through modules to check or using a second set of names for fast lookup.
     */
    template <typename DomainType, typename DomainModuleType>
    void initModule(std::string const& moduleName, Data::JsonScope const& settings, DomainType& domainReference) {
        if constexpr(std::is_same_v<DomainType, Domain>) {
            // If the DomainType is the base Domain class, we must initialize modules without scope,
            // otherwise this will lead to circular initialization problems.
            static_assert(!HasKeyGroup<DomainModuleType>, "DomainModules linked to the base Domain class cannot have a scope. Please remove the static Key::scope member from the module. Use the callers scope instead!");

            auto& scope = domainReference.domainScope.shareDummyScopeBase();
            auto DomainModule = std::make_unique<DomainModuleType>(moduleName, domainReference, funcTree, scope, settings);
            DomainModule->reinit();
            modules.push_back(std::move(DomainModule));
        } else {
            auto DomainModule = createModule<DomainType, DomainModuleType>(moduleName, settings, domainReference, getFuncTree());
            DomainModule->reinit();
            modules.push_back(std::move(DomainModule));
        }
    }

    /**
     * @brief Updates all DomainModules.
     */
    void updateModules() const ;

    /**
     * @brief Re-initializes all DomainModules.
     */
    void reinitModules() const {
        for (auto const& module : modules) {
            module->reinit();
        }
    }

    //------------------------------------------
    // Command parsing

    /**
     * @brief Parses a string into a Nebulite command and prints potential errors to stderr.
     * @details Make sure the first arg is a name and not the function itself!
     *          - `parseStr("set text Hello World")` -> does not work!
     *          - `parseStr("<someName> set text Hello World")` -> works
     *          The first argument is reserved for debugging and should be used as a way to tell the parser from where it was called:
     *          ```cpp
     *          void myFunction(){
     *              parseStr("myFunction set text Hello World");
     *          }
     *          ```
     *          If set fails, we can use the first argument `argv[0]` to identify the source of the command.
     *          The function is marked as `[[nodiscard]]` to ensure that the caller handles potential errors.
     *          The errors are not printed to stderr by default to allow the caller to handle them as needed.
     * @param str The string to parse.
     * @return Potential errors that occurred on command execution
     */
    [[nodiscard]] Constants::Event parseStr(std::string const& str);

    /**
     * @brief Necessary operations before parsing commands.
     * @details Override this function to implement domain-specific pre-parse logic.
     * @return Error code `Constants::ErrorTable::NONE()` if there was no critical stop,
     *         an error code otherwise.
     */
    [[nodiscard]] virtual Constants::Event preParse() {
        return Constants::Event::Success;
    }

    //------------------------------------------
    // Access to private members

    /**
     * @brief Gets the name of the domain.
     * @return The name of the domain.
     */
    [[nodiscard]] std::string const& getName() const { return domainName; }

    /**
     * @brief Retrieves the ordered cache list directly with minimal locking
     * @param uniqueId The unique ID for the ordered cache list.
     * @param keys The vector of keys to populate the cache with if it does not exist.
     * @return A pointer to the ordered vector of double pointers for the specified keys.
     */
    [[nodiscard]] double** ensureOrderedCacheList(uint64_t const& uniqueId, std::vector<Data::ScopedKeyView> const& keys) const ;

    /**
     * @brief Locks the domain's document for thread-safe access.
     */
    [[nodiscard]] std::unique_lock<std::recursive_mutex> lockDocument() const ;

    // Stream for collecting any output during command execution, which can be used for debugging or logging purposes.
    Utility::Capture capture;

protected:
    /**
     * @brief Offers access to the internal FuncTree for function binding.
     *        Marked as protected, as it's only used to initialize DomainModules.
     * @return A shared pointer to the internal FuncTree.
     */
    std::shared_ptr<FuncTree<Constants::Event, Domain&, Data::JsonScope&>> getFuncTree() {
        return funcTree;
    }

    /**
     * @brief Helper function to split a serialization or link with commands into tokens.
     * @param serialOrLinkWithCommands The serialization string or link with commands to split.
     * @return A vector of tokens. First token is the serialization or link, subsequent tokens are commands.
     */
    [[nodiscard]] static std::vector<std::string> stringToDeserializeTokens(std::string const& serialOrLinkWithCommands);

    /**
     * @brief Base deserialization function to be called by derived classes in their own deserialization.
     * @details This ensures that the common deserialization logic is executed.
     *          Turns the serial or link with commands into the document, parses all commands.
     *          Using this in any deserialization implementation ensures that command parsing happens at the highest level.
     *          Re-initializes all DomainModules in the JSON scope after deserialization.
     *          Supported formats:
     *          - {object} (direct JSON)
     *          - link|c1|c2|... (additional commands after link; Domain-Serialization-Piping)
     *          - {variable|t1|t2|...}|c1|c2|... (additional commands after serialization; Domain-Serialization-Piping)
     *          Where t is a transformation and c is a command to execute after deserialization.
     * @param serialOrLinkWithCommands The serialization string or link with commands to deserialize.
     */
    void baseDeserialization(std::string const& serialOrLinkWithCommands);
};
} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAIN_HPP
