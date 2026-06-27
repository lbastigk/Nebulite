#ifndef INTERACTION_EXECUTION_DOMAIN_HPP
#define INTERACTION_EXECUTION_DOMAIN_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef> // NOLINT
#include <cstdint> // NOLINT
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

// Nebulite
#include "Constants/Event.hpp"
#include "Data/Document/ScopedKey.hpp"
#include "Interaction/Execution/DomainTree.hpp"
#include "Interaction/Execution/Tasks.hpp"
#include "Module/Base/DomainModule.hpp"
#include "Module/Base/DomainModuleBase.hpp"

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

namespace Nebulite::Module::Domain {
class Initializer;
} // namespace Nebulite::Module::Domain

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
    enum class ScopeOwnership : bool {
        Owned, // Will create and own a new JsonScope (default constructor)
        Borrowed // Will be left empty
    };

    virtual ~ScopeOwnershipManager();
    explicit ScopeOwnershipManager(ScopeOwnership ownership = ScopeOwnership::Borrowed);

    ScopeOwnershipManager(ScopeOwnershipManager const&) = delete;
    ScopeOwnershipManager& operator=(ScopeOwnershipManager const&) = delete;
    ScopeOwnershipManager(ScopeOwnershipManager&&) = delete;
    ScopeOwnershipManager& operator=(ScopeOwnershipManager&&) = delete;
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

    DocumentAccessor(DocumentAccessor const&) = delete;
    DocumentAccessor& operator=(DocumentAccessor const&) = delete;
    DocumentAccessor(DocumentAccessor&&) = delete;
    DocumentAccessor& operator=(DocumentAccessor&&) = delete;

    friend class Domain;

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
    friend class Module::Domain::Initializer;

protected:
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

template<typename T>
concept HasDomainRootScopeKeyGroup = requires{
    typename T::Key;                // nested Key type exists
    { T::Key::getScope() };         // Key::getScope() exists (now it's likely that Key inherits from KeyGroup.
    { T::Key::useOutsideScope()};   // Some KeyGroups force us to pick the scope
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
     */
    std::shared_ptr<DomainTree> funcTree;

    /**
     * @brief Stores all available modules
     */
    std::vector<std::unique_ptr<Module::Base::DomainModuleBase>> modules;

    /**
     * @brief Holds identification information for the domain, including a unique ID and a hashed version of the ID for better distribution.
     * @details The values are initialized lazily to ensure that they are only generated when needed.
     */
    class Identifier {
        /**
         * @brief Generates the unique id of this domain
         * @return The id generated
         */
        static std::size_t idGenerator();

        /**
         * @brief Generates a hashed version of the domain id.
         * @details This is done for proper distribution of the id to workers.
         *          If every RenderObject has N many subdomains, the RenderObject (R) id will likely be
         *          N*R+M, meaning the modulo is always the same. That's not optimal for proper worker distribution.
         * @param x The id input
         * @return The hashed id
         */
        static std::size_t splitMix64(std::size_t x);

        void init();

        bool initialized = false;

        // Unique ID for the domain, used for ordered cache lists and other purposes
        std::size_t id = 0;

        // Hashed ID for better distribution
        std::size_t idHashed = 0;

    public:
        /**
         * @brief Gets the id of this identifier instance, potentially initializing it if it hasn't been initialized yet.
         * @return The id
         */
        std::size_t const& getId();

        /**
         * @brief Gets the hashed id of this identifier instance, potentially initializing it if it hasn't been initialized yet.
         * @return The id
         */
        std::size_t const& getIdHashed();
    } mutable identifier;



    /**
     * @brief Necessary operations before parsing commands.
     * @details Override this function to implement domain-specific pre-parse logic.
     * @return Error code `Constants::ErrorTable::NONE()` if there was no critical stop,
     *         an error code otherwise.
     */
    [[nodiscard]] virtual Constants::Event preParse() {
        return Constants::Event::Success;
    }

public:
    Domain(std::string const& name, Data::JsonScope& documentReference, Utility::IO::Capture& parentCapture);

    explicit Domain(std::string const& name, Utility::IO::Capture& parentCapture);

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

    [[nodiscard]] std::size_t const& getId() const {
        return identifier.getId();
    }

    [[nodiscard]] std::size_t const& getIdHashed() const {
        return identifier.getIdHashed();
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
     * @brief Derives the scope for a DomainModule based on the DomainType and DomainModuleType.
     * @details The scope is derived based on the following rules:
     *          1.) If the DomainType is the base Domain class, the scope is either dummy or the full scope
     *              of the domain based on the presence of the KeyGroup and the useOutsideScope flag.
     *          2.) The presence of a KeyGroup struct derived from Data::KeyGroup sets the scope
     *          3.) If none of the above applies, a dummy scope is shared (no workspace)
     * @tparam DomainType The type of Domain used
     * @tparam DomainModuleType The type of DomainModule to derive the scope for
     * @param jsonScope The scope of the domain.
     * @return A reference to the derived scope for the DomainModule.
     */
    template <typename DomainType, typename DomainModuleType>
    static auto& scopeDeriver(Data::JsonScope& jsonScope) {
        if constexpr(std::is_same_v<DomainType, Domain>) { // Base class module specialties
            static_assert(!HasKeyGroup<DomainModuleType> || HasDomainRootScopeKeyGroup<DomainModuleType>,
                "DomainModules linked to the base Domain class cannot have a predefined scope. "
                "Please remove the static Key::scope member from the module or force domain scope using Data::KeyGroup<Data::ScopedKey::domainRootScope>."
                "If possible, use the callers scope inside functions instead of sharing scopes per module!"
            );
            if constexpr (HasDomainRootScopeKeyGroup<DomainModuleType>) {
                return jsonScope.shareScope("");
            }
            else {
                return jsonScope.shareDummyScope();
            }
        }
        else if constexpr (HasKeyGroup<DomainModuleType>) {
            if constexpr (HasDomainRootScopeKeyGroup<DomainModuleType>) {
                return jsonScope.shareScope("");
            }
            else if constexpr (DomainModuleType::Key::hasScope()) {
                return jsonScope.shareScope(Data::ScopedKey(DomainModuleType::Key::getScope(), ""));
            }
            else {
                return jsonScope.shareDummyScope();
            }
        }
        else {
            return jsonScope.shareDummyScope();
        }
    }

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
    static std::unique_ptr<DomainModuleType> createModule(std::string const& moduleName, Data::JsonScope const& settings, DomainType& domainReference, std::shared_ptr<DomainTree> const& funcTree) {
        typename Module::Base::DomainModule<DomainType>::ConstructorParams params = {
            .domainReference = domainReference,
            .name = moduleName,
            .scope = scopeDeriver<DomainType, DomainModuleType>(domainReference.domainScope),
            .funcTreePtr = funcTree,
            .settings = settings
        };
        auto DomainModule = std::make_unique<DomainModuleType>(params);
        DomainModule->reinit();
        return DomainModule;
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
        modules.push_back(createModule<DomainType, DomainModuleType>(moduleName, settings, domainReference, funcTree));
    }

    /**
     * @brief Updates all DomainModules and parses the TaskQueues
     */
    void updateModules() const ;

    void parseTaskQueues(bool recover);

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
     * @param ctx The context of the caller
     * @param ctxScope The context scope of the caller
     * @return Potential errors that occurred on command execution
     */
    [[nodiscard]] Constants::Event parseStr(std::string_view str, Context& ctx, ContextScope& ctxScope) const ;

    /**
     * @brief Finds possible completions of registered functions, categories and variables for a given pattern
     * @param pattern The pattern to match for completions, full command
     * @return A vector of possible completions
     */
    [[nodiscard]] std::vector<std::string> findCompletions(std::string const& pattern) const {
        return funcTree->findCompletionForFullCommand(pattern);
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
    [[nodiscard]] double** ensureOrderedCacheList(std::uint64_t uniqueId, std::vector<Data::ScopedKeyView> const& keys) const ;

    /**
     * @brief Locks the domain's document for thread-safe access.
     */
    [[nodiscard]] std::unique_lock<std::recursive_mutex> lockDocument() const ;

    // Stream for collecting any output during command execution, which can be used for debugging or logging purposes.
    Utility::IO::Capture capture;

    // Tasks
    Tasks tasks;

    // Estimate cost of rulesets
    /**
     * @brief Estimates the computational cost of updating the Domains associated rulesets.
     *        Based on the amount of evaluations and variables in the ruleset.
     * @param onlyInternal If true, only considers internal rulesets. Defaults to true.
     * @return The estimated computational cost.
     */
    [[nodiscard]] std::uint64_t estimateComputationalCost(bool onlyInternal = true) const ;

protected:
    struct Cost {
        explicit Cost(Data::JsonScope const& scope);

        double* local;
        double* global;
    } cost;

    /**
     * @brief Offers access to the internal FuncTree for function binding.
     *        Marked as protected, as it's only used to initialize DomainModules.
     * @return A shared pointer to the internal FuncTree.
     */
    std::shared_ptr<DomainTree> getFuncTree() {
        return funcTree;
    }

    /**
     * @brief Helper function to split a serialization or link with commands into tokens.
     * @param serialOrLinkWithCommands The serialization string or link with commands to split.
     * @return A vector of tokens. First token is the serialization or link, subsequent tokens are commands.
     */
    [[nodiscard]] static std::vector<std::string> stringToDeserializeTokens(std::string_view const& serialOrLinkWithCommands);

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
#endif // INTERACTION_EXECUTION_DOMAIN_HPP
