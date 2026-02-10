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

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Interaction/Execution/FuncTree.hpp"
#include <string_view>

//------------------------------------------
// Forward declarations: Domains

namespace Nebulite::Core {
class Environment;
class GlobalSpace;
class JsonScope;
class Renderer;
class RenderObject;
class Texture;
} // namespace Nebulite::Core

//------------------------------------------
// Forward declarations: Other classes

namespace Nebulite::Data {
class MappedOrderedDoublePointers;
class TaskQueue;
} // namespace Nebulite::Data

namespace Nebulite::DomainModule {
class Initializer;
} // namespace Nebulite::DomainModule

namespace Nebulite::Interaction {
class Context; // Requires access to demote to ContextScope
} // namespace Nebulite::Interaction

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

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
/**
 * @brief DocumentAccessor provides controlled access to a domain's JSON document.
 * @details This class is designed to be a friend of various domain-related classes,
 *          allowing them to access and manipulate the domain's JSON document safely.
 *          This ensures per-class access control while maintaining encapsulation.
 *          For example, DomainModules should not have direct access to the domain's document,
 *          due to the encapsulation they provide. Instead, we pass a scope to them.
 */
class DocumentAccessor {
public:
    explicit DocumentAccessor(Core::JsonScope& d);

    virtual ~DocumentAccessor();

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
    Core::JsonScope& domainScope;

    /**
     * @brief Provides access to the domain's scoped JSON document.
     * @return Reference to the domain's JSON document.
     */
    [[nodiscard]] Data::JsonScopeBase& domainScopeBase() const ;
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
concept HasKeyScope = requires {
    typename T::Key;           // nested Key type exists
    { T::Key::scope };         // Key::scope exists (any type)
};

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
    std::shared_ptr<FuncTree<Constants::Error, Domain&, Data::JsonScopeBase&>> funcTree;

    /**
     * @brief Stores all available modules
     */
    std::vector<std::unique_ptr<DomainModuleBase>> modules;

public:
    Domain(std::string const& name, Core::JsonScope& documentReference)
        : DocumentAccessor(documentReference), domainName(name){
        funcTree = std::make_shared<FuncTree<Constants::Error, Domain&, Data::JsonScopeBase&>>(
            name,
            Constants::ErrorTable::NONE(),
            Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTIONCALL_INVALID()
        );

        // Set default preParse to Domain::preParse
        funcTree->setPreParse([this] { return preParse(); });
    }

    ~Domain() override ;

    //------------------------------------------
    // Disallow copying and moving

    Domain(Domain const&) = default;
    Domain& operator=(Domain const& other);
    Domain(Domain&&) = default;
    Domain& operator=(Domain&& other) noexcept;

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

    //------------------------------------------
    // Updating

    /**
     * @brief Updates the domain.
     *        On overwriting, make sure to update all subdomains and DomainModules as well.
     */
    virtual Constants::Error update() { return Constants::ErrorTable::NONE(); }

    //------------------------------------------
    // Module Initialization and Updating

    /**
     * @brief Initializes DomainModules based on the template parameter with proper linkage
     * @tparam DomainModuleType The type of module to initialize
     * @param moduleName The name of the module
     * @param scope The workspace JsonScope for the module
     * @param settings The settings JsonScope for the module
     * @param domainReference Reference to the domain to link the module to
     */
    template <typename DomainType, typename DomainModuleType>
    requires (!HasKeyScope<DomainModuleType>)
    [[deprecated("Please add a static member `Key::scope` to your DomainModuleType to allow automatic scope sharing in initModule(moduleName, settings, domainReference).")]]
    void initModule(std::string moduleName, Data::JsonScopeBase& scope, Data::JsonScopeBase const& settings, DomainType& domainReference) {
        auto DomainModule = std::make_unique<DomainModuleType>(moduleName, domainReference, getFuncTree(), scope, settings);
        DomainModule->reinit();
        modules.push_back(std::move(DomainModule));
    }

    // Deprecated overload for module types that DO define Key::scope: suggest the automatic scope-sharing overload
    template <typename DomainType, typename DomainModuleType>
    requires (HasKeyScope<DomainModuleType>)
    [[deprecated("DomainModuleType defines Key::scope; prefer initModule(moduleName, settings, domainReference) to automatically share the module scope.")]]
    void initModule(std::string moduleName, Data::JsonScopeBase& scope, Data::JsonScopeBase const& settings, DomainType& domainReference) {
        auto DomainModule = std::make_unique<DomainModuleType>(moduleName, domainReference, getFuncTree(), scope, settings);
        DomainModule->reinit();
        modules.push_back(std::move(DomainModule));
    }

    /**
     * @brief Initializes DomainModules based on the template parameter with proper linkage, deriving the scope from the module's static scope member.
     * @details If not member `Key::scope` is defined in the DomainModuleType, a dummy scope will be shared (no workspace)
     * @tparam DomainType The type of the domain to link the module to, used for sharing the correct scope
     * @tparam DomainModuleType The type of module to initialize, must have a static member `Key::scope` that defines the scope for the module
     * @param moduleName The name of the module
     * @param settings The settings JsonScope for the module
     * @param domainReference Reference to the domain to link the module to
     * @todo Later on we should check if the module already exists and reject initialization if it does.
     *       Either always looping through modules to check or using a second set of names for fast lookup.
     */
    template <typename DomainType, typename DomainModuleType>
    requires (HasKeyScope<DomainModuleType>)
    void initModule(std::string moduleName, Data::JsonScopeBase const& settings, DomainType& domainReference) {
        // Determine the key from root level
        static auto const scopeKey = Data::ScopedKey("", DomainModuleType::Key::scope);

        // Share the scope based on the module's defined scope
        auto& scope = domainReference.domainScope.shareScopeBase(scopeKey); // Sharing a scope based on the module's defined scope
        auto DomainModule = std::make_unique<DomainModuleType>(moduleName, domainReference, getFuncTree(), scope, settings);
        DomainModule->reinit();
        modules.push_back(std::move(DomainModule));
    }
    template <typename DomainType, typename DomainModuleType>
    requires (!HasKeyScope<DomainModuleType>)
    void initModule(std::string moduleName, Data::JsonScopeBase const& settings, DomainType& domainReference) {
        auto& scope = domainReference.domainScope.shareDummyScopeBase();
        auto DomainModule = std::make_unique<DomainModuleType>(moduleName, domainReference, getFuncTree(), scope, settings);
        DomainModule->reinit();
        modules.push_back(std::move(DomainModule));
    }



    /**
     * @brief Updates all DomainModules.
     */
    void updateModules() const {
        for (auto const& module : modules) {
            module->update();
        }
    }

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
    [[nodiscard]] Constants::Error parseStr(std::string const& str);

    /**
     * @brief Necessary operations before parsing commands.
     * @details Override this function to implement domain-specific pre-parse logic.
     * @return Error code `Constants::ErrorTable::NONE()` if there was no critical stop,
     *         an error code otherwise.
     */
    virtual Constants::Error preParse() {
        return Constants::ErrorTable::NONE();
    }

    //------------------------------------------
    // Access to private members

    /**
     * @brief Gets the name of the domain.
     * @return The name of the domain.
     */
    [[nodiscard]] std::string const& getName() const { return domainName; }

    /**
     * @brief Gets the ordered cache list map of the domain's document.
     * @return Pointer to the ordered cache list map.
     */
    [[nodiscard]] virtual Data::MappedOrderedDoublePointers* getDocumentCacheMap() const ;

    /**
     * @brief Locks the domain's document for thread-safe access.
     */
    [[nodiscard]] std::scoped_lock<std::recursive_mutex> lockDocument() const ;

protected:
    /**
     * @brief Offers access to the internal FuncTree for function binding.
     *        Marked as protected, as it's only used to initialize DomainModules.
     * @return A shared pointer to the internal FuncTree.
     */
    std::shared_ptr<FuncTree<Constants::Error, Domain&, Data::JsonScopeBase&>> getFuncTree() {
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
