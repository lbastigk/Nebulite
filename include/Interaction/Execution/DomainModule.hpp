/**
 * @file DomainModule.hpp
 * 
 * This file defines the DomainModule class, which extends the functionality of the FuncTree
 * class to support category-based function bindings.
 */

#pragma once

/**
 * @brief Macro to define a new Nebulite DomainModule class.
 * 
 * @todo Insert macro into all DomainModules
 */
#define NEBULITE_DOMAINMODULE(DomainName,DomainModuleName) \
    class DomainModuleName : public Nebulite::Interaction::Execution::DomainModule<DomainName>

#define NEBULITE_DOMAINMODULE_CONSTRUCTOR(DomainName,DomainModuleName) \
    DomainModuleName(std::string moduleName, DomainName* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>* funcTreePtr) \
    : DomainModule(moduleName, domain, funcTreePtr)

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/FuncTree.hpp"

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
 * @brief Wrapper class for binding functions to a specific category in the FuncTree.
 * 
 * This allows for cleaner separation of object files for different categories
 * and reduces boilerplate code when attaching functions to the FuncTree.
 * 
 * Within the Core FuncTree, Categories are initialized with references to the funcTree,
 * invokeand the GlobalSpace, allowing them to individually bind functions on construction.
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
    DomainModule(std::string moduleName, DomainType* domain, FuncTree<Nebulite::Constants::Error>* funcTreePtr)
        : moduleName(moduleName), domain(domain), funcTree(funcTreePtr) {}

    /**
     * @brief Virtual update function to be Overwridden by derived classes.
     */
    virtual void update() = 0;

    /**
     * @brief Binds a member function to the FuncTree.
     * 
     * This function template allows for binding member functions of any class type
     * to the FuncTree, automatically handling the necessary type conversions.
     * 
     * Make sure the function has the signature:
     * ```cpp
     * Error functionName(int argc, char** argv);
     * ```
     *
     * @tparam ClassType The type of the class containing the member function.
     * @param method A pointer to the member function to bind.
     * @param name The name to associate with the bound function.
     */
    template<typename ClassType>
    void bindFunction(Nebulite::Constants::Error (ClassType::*method)(int, char**),const std::string& name,const std::string& help) {
        // Automatically pass 'this' (the derived class instance) to bindFunction
        funcTree->bindFunction(
            static_cast<ClassType*>(this),  // Auto-cast to correct type
            method,                         // Member function pointer
            name, 
            help
        );
    }

    bool bindSubtree(const std::string& name, const std::string& description) {
        return funcTree->bindSubtree(name, description);
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
    void bindVariable(std::string* variablePtr,const std::string& name,const std::string& help) {
        // Bind a variable to the FuncTree
        funcTree->bindVariable(variablePtr, name, help);
    }

    // Prevent copying
    DomainModule(const DomainModule&) = delete;
    DomainModule& operator=(const DomainModule&) = delete;

protected:
    //------------------------------------------
    // Name of Module
    std::string moduleName;
    
    //------------------------------------------
    // Linkages
    DomainType* domain;                                     // Workspace of the DomainModule

private:
    FuncTree<Nebulite::Constants::Error>* funcTree;         // Where to bind the expanded functions
};
}   // namespace Interaction
}   // namespace Execution
}   // namespace Nebulite