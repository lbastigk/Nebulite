/**
 * @file DomainModule.hpp
 * @brief This file defines the DomainModule class, which extends the functionality of the FuncTree
 *        class to support category-based function bindings.
 */

#ifndef NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_HPP
#define NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_HPP

//------------------------------------------
// Macro for DomainModule definition

bool constexpr endsWithNewline(std::string_view str) {
    return !str.empty() && str.back() == '\n';
}

#define NEBULITE_DOMAINMODULE(DomainName,DomainModuleName) \
    class DomainModuleName final : public Nebulite::Interaction::Execution::DomainModule<DomainName>

#define NEBULITE_DOMAINMODULE_CONSTRUCTOR(DomainName,DomainModuleName) \
    explicit DomainModuleName(std::string const& name, DomainName& domainReference, std::shared_ptr<Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>> funcTreePtr) \
    : DomainModule(name, domainReference, std::move(funcTreePtr))

#define BINDFUNCTION(func, name,desc) \
    static_assert(endsWithNewline(desc), "Function description must end with a newline character."); \
    bindFunction(func, name, desc)


//------------------------------------------
// Includes

// Standard library
#include <utility>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Data/Document/JsonScope.hpp"
#include "Interaction/Execution/FuncTree.hpp"

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
     * @details The constructor initializes the DomainModuleBase with
     *          the FuncTree pointer for binding functions and variables.
     */
    explicit DomainModuleBase(std::shared_ptr<FuncTree<Constants::Error>> funcTreePtr)
        : funcTree(std::move(funcTreePtr)) {}

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
     * @todo Mark as [[nodiscard]] and refactor usage accordingly
     */
    bool bindCategory(std::string_view const& name, std::string_view const& helpDescription) const {
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

protected:
    /**
     * @brief Pointer to the internal FuncTree for binding functions and variables.
     * @details We need a pointer here to avoid circular dependencies that are hard to resolve,
     *          as both Domain and DomainModule are templated classes
     *          FuncTree, however, is fully defined at this point, so we can use it directly.
     *          Instead of making a mess by untangling the templates, we simply use a pointer
     *          to the non-templated interface.
     */
    std::shared_ptr<FuncTree<Constants::Error>> funcTree;
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
     * @todo TODO: Add argument for custom scope prefix
     */
    DomainModule(std::string name, DomainType& domainReference, std::shared_ptr<FuncTree<Constants::Error>> funcTreePtr);

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
