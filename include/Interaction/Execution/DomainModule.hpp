/**
 * @file DomainModule.hpp
 * 
 * This file defines the DomainModule class, which extends the functionality of the FuncTree
 * class to support category-based function bindings.
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
#include <utility>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/FuncTree.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Execution {
/**
 * @class Nebulite::Interaction::Execution::DomainModule
 * @brief Wrapper class for binding functions to a specific category in the FuncTree and adding separate update routines.
 *        This allows for cleaner separation of object files for different categories
 *        and reduces boilerplate code when attaching functions to the FuncTree.
 */
template <typename DomainType>
class DomainModule {
public:
    /**
     * @brief Constructor for the DomainModule base class.
     *        The constructor initializes the DomainModule with a reference to the domain and
     *        the FuncTree.
     */
    DomainModule(
        std::string name,
        DomainType* domainPtr,
        std::shared_ptr<FuncTree<Constants::Error>> funcTreePtr
        )
        : moduleName(std::move(name)), domain(domainPtr), funcTree(std::move(funcTreePtr)) {
    }

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
     * @brief Static helper function to bind a member function to a given FuncTree.
     * @tparam ClassType The type of the class containing the member function.
     * @tparam FuncTreeType The type of the FuncTree to bind the function to.
     * @tparam ReturnType The return type of the member function, must match the FuncTree's return type.
     * @tparam Args The argument types of the member function, must match the FuncTree's additional argument types.
     * @param tree Pointer to the FuncTree to bind the function to.
     * @param obj Pointer to the object instance containing the member function.
     * @param methodPtr Pointer to the member function to bind.
     * @param name Name to associate with the bound function.
     * @param helpDescription Pointer to a string containing the help description for the function.
     */
    template <typename ClassType, typename FuncTreeType, typename ReturnType, typename... Args>
    void bindFunctionStatic(FuncTreeType* tree, ClassType* obj, ReturnType (ClassType::*methodPtr)(Args...), std::string const& name, std::string const* helpDescription) {
        using MemberVariant = FuncTreeType::template MemberMethod<ClassType>;
        MemberVariant methodVariant{methodPtr}; // Wrap the member function pointer in the variant
        std::visit([&](auto mpr) {
            // Dispatch to the actual funcTree->bindFunction using std::visit
            tree->bindFunction(
                obj,
                MemberVariant(mpr),
                name,
                helpDescription
                );
        }, methodVariant);
    }

    /**
    * @brief Binds a member function to the FuncTree.
     *       This function template allows for binding member functions of any class type
     *       to the FuncTree, automatically handling the necessary type conversions.
     *       This function is a wrapper around the static bindFunctionStatic helper for methods inside the DomainModule.
     * @tparam ClassType The type of the class containing the member function.
     * @tparam ReturnType The return type of the member function.
     * @tparam Args The argument types of the member function.
     * @param methodPtr A pointer to the member function to bind.
     * @param name The name to associate with the bound function.
     * @param helpDescription A pointer to a string containing the help description for the function.
     */
    template <typename ClassType, typename ReturnType, typename... Args>
    void bindFunction(ReturnType (ClassType::*methodPtr)(Args...), std::string const& name, std::string const* helpDescription) {
        bindFunctionStatic(funcTree.get(), static_cast<ClassType*>(this), methodPtr, name, helpDescription);
    }

    /**
     * @brief Binds a category to the FuncTree.
     * 
     * A category acts as a "function bundler" to the main tree.
     * 
     * @param name Name of the category
     * @param helpDescription Description of the category, shown in the help command. First line is shown in the general help, full description in detailed help
     * @return true if the category was created successfully, false if a category with the same name already exists
     */
    bool bindCategory(std::string const& name, std::string const* helpDescription) const {
        return funcTree->bindCategory(name, helpDescription);
    }

    /**
     * @brief Binds a variable to the command tree.
     * 
     * Make sure the variable is of type std::string*.
     * 
     * Once bound, it can be set via command line arguments: --varName=value (Must be before the function name!)
     * 
     * A simple argument of '--varName' will set the value to "true"
     */
    void bindVariable(bool* variablePtr, std::string const& name, std::string const* helpDescription) const {
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
     * 
     * We need a pointer here to avoid circular dependencies that are hard to resolve,
     * as both Domain and DomainModule are templated classes
     * 
     * FuncTree, however, is fully defined at this point, so we can use it directly.
     * 
     * Instead of making a mess by untangling the templates, we simply use a pointer
     * to the non-templated interface.
     */
    std::shared_ptr<FuncTree<Constants::Error>> funcTree;
};
} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_HPP
