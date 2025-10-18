/**
 * @file DomainModule.hpp
 * 
 * This file defines the DomainModule class, which extends the functionality of the FuncTree
 * class to support category-based function bindings.
 */

#pragma once
//------------------------------------------
// Macro for DomainModule definition

#define NEBULITE_DOMAINMODULE(DomainName,DomainModuleName) \
    class DomainModuleName : public ::Nebulite::Interaction::Execution::DomainModule<DomainName>

#define NEBULITE_DOMAINMODULE_CONSTRUCTOR(DomainName,DomainModuleName) \
    DomainModuleName(::std::string moduleName, DomainName* domain, ::Nebulite::Interaction::Execution::FuncTree<::Nebulite::Constants::Error>* funcTreePtr, Nebulite::Core::GlobalSpace* globalSpace) \
    : DomainModule(moduleName, domain, funcTreePtr, globalSpace)

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/FuncTree.hpp"

//------------------------------------------
// Pre-declarations
namespace Nebulite::Core{
    class GlobalSpace;
}

//------------------------------------------
// Helper to detect member function existence
template <typename T>
class has_setupBindings {
    typedef char yes[1];
    typedef char no[2];
    template <typename U, void (U::*)()> struct SFINAE {};
    template <typename U> static yes& test(SFINAE<U, &U::update>*);
    template <typename U> static no& test(...);
public:
    static constexpr bool value = sizeof(test<T>(0)) == sizeof(yes);
};
template <typename T>
class has_update {
    typedef char yes[1];
    typedef char no[2];
    template <typename U, void (U::*)()> struct SFINAE {};
    template <typename U> static yes& test(SFINAE<U, &U::update>*);
    template <typename U> static no& test(...);
public:
    static constexpr bool value = sizeof(test<T>(0)) == sizeof(yes);
};


namespace Nebulite{
namespace Interaction{
namespace Execution{
//------------------------------------------
/**
 * @class Nebulite::Interaction::Execution::DomainModule
 * @brief Wrapper class for binding functions to a specific category in the FuncTree and adding separate update routines.
 * 
 * This allows for cleaner separation of object files for different categories
 * and reduces boilerplate code when attaching functions to the FuncTree.
 */
template<typename DomainType>
class DomainModule{
public:
    /**
     * @brief Constructor for the DomainModule base class.
     * 
     * The constructor initializes the DomainModule with a reference to the domain and
     * the FuncTree.
     */
    DomainModule(std::string moduleName, DomainType* domain, FuncTree<Nebulite::Constants::Error>* funcTreePtr, Nebulite::Core::GlobalSpace* globalSpace)
        : moduleName(moduleName), domain(domain), global(globalSpace), funcTree(funcTreePtr) {}

    /**
     * @brief Virtual update function to be Overwridden by derived classes.
     */
    virtual Nebulite::Constants::Error update() { return Nebulite::Constants::ErrorTable::NONE(); };

    /**
     * @brief Binds a member function to the FuncTree.
     * 
     * This function template allows for binding member functions of any class type
     * to the FuncTree, automatically handling the necessary type conversions.
     * 
     * Make sure the function has the signature:
     * ```cpp
     * Error functionName([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]);
     * ```
     *
     * @tparam ClassType The type of the class containing the member function.
     * @param method A pointer to the member function to bind.
     * @param name The name to associate with the bound function.
     */
    template<typename ClassType>
    void bindFunction(Nebulite::Constants::Error (ClassType::*method)(int, char**),const std::string& name, const std::string* helpDescription) {
        // Automatically pass 'this' (the derived class instance) to bindFunction
        funcTree->bindFunction(
            static_cast<ClassType*>(this),  // Auto-cast to correct type
            method,                         // Member function pointer
            name, 
            helpDescription
        );
    }

    /**
     * @brief Binds a subtree to the FuncTree.
     * 
     * A subtree acts as a "function bundler" to the main tree.
     * 
     * @param name Name of the subtree
     * @param description Description of the subtree, shown in the help command. First line is shown in the general help, full description in detailed help
     * @return true if the subtree was created successfully, false if a subtree with the same name already exists
     */
    bool bindSubtree(const std::string& name, const std::string* helpDescription) {
        return funcTree->bindSubtree(name, helpDescription);
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
    void bindVariable(bool* variablePtr, const std::string& name, const std::string* helpDescription) {
        // Bind a variable to the FuncTree
        funcTree->bindVariable(variablePtr, name, helpDescription);
    }

    // Prevent copying
    DomainModule(const DomainModule&) = delete;

    // Prevent assignment
    DomainModule& operator=(const DomainModule&) = delete;

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

    /**
     * @brief Pointer to the global space of the DomainModule
     */
    Nebulite::Core::GlobalSpace* global;

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
    FuncTree<Nebulite::Constants::Error>* funcTree;
};
}   // namespace Interaction
}   // namespace Execution
}   // namespace Nebulite