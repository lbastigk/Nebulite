/**
 * @file DomainModule.hpp
 * @brief This file defines the DomainModule class, which extends the functionality of the FuncTree
 *        class to support category-based function bindings.
 */

#ifndef NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_HPP
#define NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_HPP

//------------------------------------------
// Macro for DomainModule definition

#define NEBULITE_DOMAINMODULE(DomainName,DomainModuleName) \
    class DomainModuleName final : public Nebulite::Interaction::Execution::DomainModule<DomainName>

#define NEBULITE_DOMAINMODULE_CONSTRUCTOR(DomainName,DomainModuleName) \
    explicit DomainModuleName(std::string const& name, DomainName* domainPtr, std::shared_ptr<Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>> funcTreePtr) \
    : DomainModule(name, domainPtr, std::move(funcTreePtr))

//------------------------------------------
// Includes

// Standard library
#include <type_traits>
#include <utility>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/FuncTree.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Execution {

/**
 * @class Nebulite::Interaction::Execution::DomainModule
 * @brief Wrapper class for binding functions to a specific category in the FuncTree and adding separate update routines.
 * @details Allows for cleaner separation of object files for different categories
 *          and reduces boilerplate code when attaching functions to the FuncTree.
 */
template <typename DomainType>
class DomainModule {
public:
    /**
     * @brief Constructor for the DomainModule base class.
     * @details The constructor initializes the DomainModule with a reference to the domain and
     *          the FuncTree.
     */
    DomainModule(
        std::string name,
        DomainType* domainPtr,
        std::shared_ptr<FuncTree<Constants::Error>> funcTreePtr
    ) : moduleName(std::move(name)), domain(domainPtr), funcTree(std::move(funcTreePtr)) {}

    /**
     * @brief Virtual destructor for DomainModule.
     */
    virtual ~DomainModule() = default;

    /**
     * @brief Virtual update function to be Overwritten by derived classes.
     */
    virtual Constants::Error update() { return Constants::ErrorTable::NONE(); }

    /**
     * @brief Virtual re-initialization function to be Overwritten by derived classes.
     */
    virtual void reinit() {}

    /**
     * @brief Binds a static/free function to the FuncTree.
     * @details This function template allows for binding static or free functions
     *          to the FuncTree, automatically handling the necessary type conversions.
     * @tparam FuncTreeType The type of the FuncTree to bind to.
     * @tparam ReturnType The return type of the function.
     * @tparam Args The argument types of the function.
     * @param functionPtr A pointer to the static/free function to bind.
     * @param name The name to associate with the bound function.
     * @param helpDescription The help description for the function.
     *                        First line is shown in the general help, full description in detailed help
     */
    template <typename FuncTreeType, typename ReturnType, typename... Args>
    void bindFunctionStatic(
        FuncTreeType* tree,
        ReturnType (*functionPtr)(Args...),
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    /**
     * @brief Binds a member function to the FuncTree.
     * @details This function template allows for binding member functions
     *          to the FuncTree, automatically handling the necessary type conversions.
     * @tparam Func The type of the member function pointer.
     * @param methodPtr A pointer to the member function to bind.
     * @param name The name to associate with the bound function.
     * @param helpDescription The help description for the function.
     *                        First line is shown in the general help, full description in detailed help
     */
    template <typename Func>
    void bindFunction(
        Func methodPtr,
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    template <typename ClassType, typename ReturnType, typename... Args>
    void bindFunction(
        ClassType* obj,
        ReturnType (ClassType::*methodPtr)(Args...),
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    template <typename ClassType, typename ReturnType, typename... Args>
    void bindFunction(
        ClassType* obj,
        ReturnType (ClassType::*methodPtr)(Args...) const,
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    /**
     * @brief Binds a category to the FuncTree.
     * @details A category acts as a "function bundler" to the main tree.
     * @param name Name of the category
     * @param helpDescription Description of the category, shown in the help command.
     *                        First line is shown in the general help, full description in detailed help
     * @return true if the category was created successfully, false if a category with the same name already exists
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
    DomainType* domain;

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
};
} // namespace Nebulite::Interaction::Execution
#include "Interaction/Execution/DomainModule.tpp"
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_HPP
