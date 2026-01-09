/**
 * @file Domain.hpp
 * @brief This file defines the Domain class, which serves as a base class for creating a Nebulite domain.
 *        The Domain class is split into a templated Domain class and a non-templated DomainBase class.
 *        The non-templated DomainBase class holds all common functionality for domains that do not require template parameters.
 */

#ifndef NEBULITE_INTERACTION_EXECUTION_DOMAIN_HPP
#define NEBULITE_INTERACTION_EXECUTION_DOMAIN_HPP

//------------------------------------------
// Macro to define a new Nebulite Domain class

#define NEBULITE_DOMAIN(DomainName) \
    class DomainName : public Nebulite::Interaction::Execution::Domain<DomainName>

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Interaction/Execution/FuncTree.hpp"

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
} // namespace Nebulite::Data

namespace Nebulite::DomainModule {
class Initializer;
} // namespace Nebulite::DomainModule

namespace Nebulite::Interaction {
class ContextBase; // Requires access to demote to ContextScope
} // namespace Nebulite::Interaction

namespace Nebulite::Interaction::Logic {
class Assignment;   // Requires access to set target documents
class Expression;   // Requires access to get unscoped values from global scope
} // namespace Nebulite::Interaction::Logic

namespace Nebulite::Interaction::Rules {
class Ruleset;
class JsonRuleset;
class StaticRuleset;

namespace Construction {
class RulesetCompiler;
} // namespace Construction
} // namespace Nebulite::Interaction::Rules

//------------------------------------------
// Document Accessor

namespace Nebulite::Interaction::Execution {
template<typename> class Domain;

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
    explicit DocumentAccessor(Core::JsonScope& d) : domainScope(d) {}

    virtual ~DocumentAccessor();

    template<typename> friend class Domain;

    friend class DomainBase;

    // Allow other Domains to access documents from each other.
    friend class Core::Environment;
    friend class Core::GlobalSpace;
    friend class Core::Renderer;
    friend class Core::RenderObject;
    friend class Core::Texture;

    // Allow ContextBase to demote to ContextScope
    friend class Nebulite::Interaction::ContextBase;

    // Assignments and Expressions need access to set/get document values
    friend class Logic::Assignment;
    friend class Logic::Expression;

    // The entire Ruleset system needs access as well
    // TODO: If we replace the Context system with ContextScope,
    //       Meaning we pass scopes instead of DomainBases,
    //       we may get rid of these friends here.
    friend class Rules::Ruleset;
    friend class Rules::JsonRuleset;
    friend class Rules::StaticRuleset;
    friend class Rules::Construction::RulesetCompiler;

    // Initializer needs access to share scopes
    friend class ::Nebulite::DomainModule::Initializer;

private:
    /**
     * @brief Each domain uses a JSON document to store its data.
     */
    Core::JsonScope& domainScope;
};
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
// Domain Base

namespace Nebulite::Interaction::Execution {

/**
 * @class DomainBase
 * @brief Non-templated base class for all Nebulite domains.
 *        Holds all common functionality for domains that do not require template parameters.
 *        This allows for a simplified interface for accessing common domain functionality.
 */
class DomainBase : public DocumentAccessor {
private:
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
    std::shared_ptr<FuncTree<Constants::Error>> funcTree;
public:
    DomainBase(std::string const& name, Core::JsonScope& documentReference)
        : DocumentAccessor(documentReference), domainName(name){
        funcTree = std::make_shared<FuncTree<Constants::Error>>(
            name,
            Constants::ErrorTable::NONE(),
            Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTIONCALL_INVALID()
        );

        // Set default preParse to DomainBase::preParse
        funcTree->setPreParse([this] { return preParse(); });
    }

    ~DomainBase() override;

    //------------------------------------------
    // Disallow copying and moving

    DomainBase(DomainBase const&) = default;
    DomainBase& operator=(DomainBase const& other);
    DomainBase(DomainBase&&) = default;
    DomainBase& operator=(DomainBase&& other) noexcept;

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
    void inherit(DomainBase const* toInheritFrom) const {
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
     * @todo Currently, this method is accessible to all DomainModules.
     *       This is a security risk, as DomainModules can execute commands outside of their scope.
     *       For this to be resolved, we need some restricted parsing mode that only allows commands
     *       to execute with data in its own scope.
     *       But requires careful design, perhaps with the scope itself as argument.
     *       Idea: parseStr(str, scope) -> foo(args, scope)
     *       Once all domainmodule functions are modernized with std::span<std::string const> args,
     *       implement a restricted parseStr version as described above,
     *       by passing around the scope as reference.
     *       Note that only modern arguments support passing additional arguments like scope!
     *       Update the base Domain functree to take the scope as additional argument.
     *       Most getDoc() calls are then replaced with the passed scope.
     *       This, however, complicates things as we need to distinguish between:
     *       - The general workspace of the domainmodule for its update routine or variable storage
     *       - The scope passed to functions for command execution
     *       For example, the simpledata domainmodule needs to use the passed scope to modify data at the correct scope,
     *       but the time domainmodule needs to use its own workspace to store the time variable.
     *       The following scopes are then relevant:
     *       - domainScope: The overall document of the domain
     *       - moduleScope: The workspace of the domainmodule
     *       - callerScope: The scope passed to functions for command execution
     *       - settingsScope: The settings scope of the domainmodule
     */
    [[nodiscard]] Constants::Error parseStr(std::string const& str) const {
        return funcTree->parseStr(str);
    }

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
    std::shared_ptr<FuncTree<Constants::Error>> getFuncTree() {
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
     *        This ensures that the common deserialization logic is executed.
     *        Turns the serial or link with commands into the document, parses all commands.
     *        Using this in any deserialization implementation ensures that command parsing happens at the highest level.
     *        Re-initializes all DomainModules in the JSON scope after deserialization.
     * @param serialOrLinkWithCommands The serialization string or link with commands to deserialize.
     */
    void baseDeserialization(std::string const& serialOrLinkWithCommands) const ;
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
template <typename DomainType>
class Domain : public DomainBase {
private:
    /**
     * @brief Stores all available modules
     */
    std::vector<std::unique_ptr<DomainModule<DomainType>>> modules;

    /**
     * @brief Name of the domain type
     */
    std::string domainName;

    //------------------------------------------
    // Inner references

    /**
     * @brief Reference to the domain itself
     *        Used to initialize DomainModules with a reference to the domain
     */
    DomainType& domain;

public:
    Domain(std::string const& name, DomainType& domainTypeReference, Core::JsonScope& documentReference)
        : DomainBase(name, documentReference), domainName(name), domain(domainTypeReference) {
    }

    //------------------------------------------
    // Disallow copying and moving

    Domain(Domain const&) = default;
    Domain& operator=(Domain const&) = default;
    Domain(Domain&&) = default;
    Domain& operator=(Domain&&) = default;

    //------------------------------------------
    // Module Initialization and Updating

    /**
     * @brief Factory method for creating DomainModule instances with proper linkage
     * @tparam DomainModuleType The type of module to initialize
     * @param moduleName The name of the module
     * @param scope The workspace JsonScope for the module
     * @param settings The prefix for the settings scope of the module
     */
    template <typename DomainModuleType>
    void initModule(std::string moduleName, Data::JsonScopeBase& scope, std::string const& settings) {
        auto DomainModule = std::make_unique<DomainModuleType>(moduleName, domain, getFuncTree(), scope, settings);
        DomainModule->reinit();
        modules.push_back(std::move(DomainModule));
    }

    /**
     * @brief Updates all DomainModules.
     */
    void updateModules() {
        for (auto& module : modules) {
            module->update();
        }
    }

    /**
     * @brief Re-initializes all DomainModules.
     */
    void reinitModules() {
        for (auto& module : modules) {
            module->reinit();
        }
    }

    /**
     * @brief Gets the name of the domain.
     * @return The name of the domain.
     */
    [[nodiscard]] std::string const& getDomainName() const {
        return domainName;
    }
};


} // namespace Nebulite::Interaction::Execution

#endif // NEBULITE_INTERACTION_EXECUTION_DOMAIN_HPP

