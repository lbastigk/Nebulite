/**
 * @file DomainModule.hpp
 * @brief This file defines the DomainModule class, which extends the functionality of the FuncTree
 *        class to support category-based function bindings.
 */

#ifndef NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_HPP
#define NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/FuncTree.hpp"
#include "Data/Document/JsonScopeBase.hpp"

//------------------------------------------
// Macro for DomainModule definition

bool constexpr endsWithNewline(std::string_view const& str) {
    return !str.empty() && str.back() == '\n';
}

#define NEBULITE_DOMAINMODULE(DomainName,DomainModuleName) \
    class DomainModuleName final : public Nebulite::Interaction::Execution::DomainModule<DomainName>

#define NEBULITE_DOMAINMODULE_CONSTRUCTOR(DomainName,DomainModuleName) \
    explicit DomainModuleName( \
        std::string const& name, DomainName& domainReference, \
        std::shared_ptr<Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>> funcTreePtr, \
        Data::JsonScopeBase& w, \
        std::string const& s \
    ) \
    : DomainModule(name, domainReference, std::move(funcTreePtr), w, s)

// Common macro to create a floating DomainModule with proper linkage
// Floating DomainModules are handled separately from regular DomainModules
// They are not updated automatically via the domains updateModules function.
// However, they offer the same "separation of concerns" as regular DomainModules
// without the additional overhead if we were to turn them into full Domains.
// Useful for small "runners" with neatly separated functionality, that need the ability to be called
// separately.
#define NEBULITE_FLOATING_DOMAINMODULE(DomainModule, DomainModuleName, Document, Workspace, Settings) \
    std::make_unique<DomainModule>( \
        #DomainModuleName, \
        *this, \
        getFuncTree(), \
        Document.domainScope.shareScopeBase(Workspace), \
        Settings \
    )

#define BINDFUNCTION(func, name,desc) \
static_assert(endsWithNewline(desc), "Function description must end with a newline character."); \
    bindFunction(func, name, desc)

//------------------------------------------
namespace Nebulite::Interaction::Execution {
/**
 * @class Nebulite::Interaction::Execution::DomainModuleBase
 * @brief Base class for DomainModule to handle common functionality.
 * @details This class provides the foundational functionality for binding functions
 *          and variables to the FuncTree, allowing derived classes to focus on
 *          domain-specific implementations.
 */
class DomainModuleBase {
public:
    /**
     * @brief Constructor for the DomainModule base class.
     * @param funcTreePtr Shared pointer to the FuncTree for binding functions and variables.
     * @param w Reference to a JsonScopeBase document for this module to use as workspace.
     * @param s String prefix for this module to use as settings.
     * @details The constructor initializes the DomainModuleBase with
     *          the FuncTree pointer for binding functions and variables.
     */
    explicit DomainModuleBase(std::shared_ptr<FuncTree<Constants::Error>> funcTreePtr, Data::JsonScopeBase& w, std::string const& s);

    //------------------------------------------
    // Static Binding Functions

    template <typename Func,typename FuncTreeType>
    static void bindFunctionStatic(
        FuncTreeType* tree,
        Func functionPtr,
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    template <typename Obj, typename Func, typename FuncTreeType>
    static void bindFunctionStatic(
        FuncTreeType* tree,
        Obj* objectPtr,
        Func functionPtr,
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    //------------------------------------------
    // Non-static Binding Functions

    template <typename R, typename C, typename... Ps>
    void bindFunction(
        R (C::*functionPtr)(Ps...),
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    template <typename R, typename C, typename... Ps>
    void bindFunction(
        R (C::*functionPtr)(Ps...) const,
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    template <typename Func>
    void bindFunction(
        Func functionPtr,
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    //------------------------------------------
    // Category and Variable Binding

    /**
     * @brief Binds a category to the FuncTree.
     * @details A category acts as a "function bundler" to the main tree.
     * @param name Name of the category
     * @param helpDescription Description of the category, shown in the help command.
     *                        First line is shown in the general help, full description in detailed help
     * @return true if the category was created successfully, false if a category with the same name already exists
     */
    [[nodiscard]] bool bindCategory(std::string_view const& name, std::string_view const& helpDescription) const {
        return funcTree->bindCategory(name, helpDescription);
    }

    /**
     * @brief Binds a variable to the command tree.
     * @details Once bound, it can be set via command line arguments: --varName=value (Must be before the function name!)
     *          A simple argument of '--varName' will set the value to "true"
     * @param variablePtr Pointer to the variable to bind.
     * @param name Name of the variable in the command tree.
     * @param helpDescription Description of the variable, shown in the help command.
     */
    void bindVariable(bool* variablePtr, std::string_view const& name, std::string_view const& helpDescription) const {
        // Bind a variable to the FuncTree
        funcTree->bindVariable(variablePtr, name, helpDescription);
    }

    /**
     * @brief Retrieves the JsonScopeBase document based on what was shared with the DomainModule.
     * @return Reference to the JsonScopeBase document.
     */
    [[nodiscard]] Data::JsonScopeBase& getDoc() const ;

    /**
     * @brief Retrieves the settings scope based on what was shared with the DomainModule.
     * @return Reference to the settings scope.
     */
    [[nodiscard]] Data::JsonScopeBase const& settings() const ;

private:
    /**
     * @brief Pointer to the internal FuncTree for binding functions and variables.
     * @details We need a pointer here to avoid circular dependencies that are hard to resolve,
     *          as both Domain and DomainModule are templated classes
     *          FuncTree, however, is fully defined at this point, so we can use it directly.
     *          Instead of making a mess by untangling the templates, we simply use a pointer
     *          to the non-templated interface.
     */
    std::shared_ptr<FuncTree<Constants::Error>> funcTree;

    /**
     * @brief Reference to the JsonScopeBase document.
     * @details This allows derived DomainModules to access and manipulate
     *          the JSON document as needed.
     */
    Data::JsonScopeBase& moduleScope;

    /**
     * @brief Reference to the settings JsonScopeBase document.
     * @details This allows derived DomainModules to access and manipulate
     *          the settings JSON document as needed.
     */
    std::string const& settingsPrefix;
};

/**
 * @class Nebulite::Interaction::Execution::DomainModule
 * @brief Wrapper class for binding functions to a specific category in the FuncTree and adding separate update routines.
 * @details Allows for cleaner separation of object files for different categories
 *          and reduces boilerplate code when attaching functions to the FuncTree.
 */
template <typename DomainType>
class DomainModule : public DomainModuleBase {
public:
    /**
     * @brief Constructor for the DomainModule base class.
     * @details The constructor initializes the DomainModule with a reference to the domain and
     *          the FuncTree.
     * @param name Name of the DomainModule, useful for debugging and logging.
     * @param domainReference Reference to the Domain instance this module is associated with.
     * @param funcTreePtr Shared pointer to the FuncTree for binding functions and variables.
     * @param scope JsonScopeBase reference for this module to use as workspace.
     * @param settings Prefix for this module to use as settings. Already scoped within "settings." in globalspace.
     */
    DomainModule(
        std::string name,
        DomainType& domainReference,
        std::shared_ptr<FuncTree<Constants::Error>> funcTreePtr,
        Data::JsonScopeBase& scope,
        std::string const& settings
    );

    /**
     * @brief Virtual destructor for DomainModule.
     */
    virtual ~DomainModule();

    /**
     * @brief Virtual update function to be Overwritten by derived classes.
     */
    virtual Constants::Error update() { return Constants::ErrorTable::NONE(); }

    /**
     * @brief Virtual re-initialization function to be Overwritten by derived classes.
     */
    virtual void reinit() {}

    // Prevent copying
    DomainModule(DomainModule const&) = delete;

    // Prevent assignment
    DomainModule& operator=(DomainModule const&) = delete;

protected:
    //------------------------------------------
    // Name of Module

    /**
     * @brief Name of the DomainModule, useful for debugging and logging.
     */
    std::string moduleName;

    //------------------------------------------
    // Linkages

    /**
     * @brief Workspace of the DomainModule
     */
    DomainType& domain;
};
} // namespace Nebulite::Interaction::Execution
#include "Interaction/Execution/DomainModule.tpp"
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_HPP
