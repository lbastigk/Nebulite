/*
This wrapper class extends the functionality of a general FuncTreeWrapper for the GlobalSpaceTree
by providing a specific implementation for category-related function bindings.

This allows for cleaner separation of object files for different categories
and reduces boilerplate code when attaching functions to the FuncTree.

Within the Core FuncTree, Categories are initialized with references to the funcTree,
invokeand the GlobalSpace, allowing them to individually bind functions on construction.

*/

/**
 * @file DomainModuleWrapper.hpp
 * 
 * This file defines the DomainModuleWrapper class, which extends the functionality of the FuncTree
 * class to support category-based function bindings.
 */

#pragma once

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
 * @class Nebulite::Interaction::Execution::DomainModuleWrapper
 * @brief Wrapper class for binding functions to a specific category in the FuncTree.
 * 
 * This allows for cleaner separation of object files for different categories
 * and reduces boilerplate code when attaching functions to the FuncTree.
 * 
 * Within the Core FuncTree, Categories are initialized with references to the funcTree,
 * invokeand the GlobalSpace, allowing them to individually bind functions on construction.
 * 
 * @todo no more derivedclass, instead we simply override the constructor. 
 * This way, we should be able to create vectors
 * of DomainModuleWrapper instances for each category.
 */
template<typename DomainType, typename DerivedClass>
class DomainModuleWrapper{
public:
    /**
     * @brief Constructor for the Wrapper class.
     * 
     * This constructor initializes the Wrapper with the given domain and FuncTree pointers,
     * and sets up the function bindings for the category.
     * 
     * The update method of the derived class is called upon construction.
     * 
     * @note A virtual function for setupBindings and update would be better so that we know that the derived class has to implement it
     * However, this would call a pure virtual function during construction as the derived class is not fully formed, which is not allowed.
     * Instead, we do a pseudo-virtual call by using static_cast to call the derived class's methods upon construction.
     */
    DomainModuleWrapper(DomainType* domain, FuncTree<Nebulite::Constants::ERROR_TYPE>* funcTreePtr)
        : domain(domain), funcTree(funcTreePtr) 
    {
        // Initialize the defined Variable and Function Bindings
        static_cast<DerivedClass*>(this)->setupBindings();

        // We call the derived class' update function to ensure it exists
        static_cast<DerivedClass*>(this)->update();
    }
    
    /**
     * @brief Binds a member function to the FuncTree.
     * 
     * This function template allows for binding member functions of any class type
     * to the FuncTree, automatically handling the necessary type conversions.
     * 
     * Make sure the function has the signature:
     * ```cpp
     * ERROR_TYPE functionName(int argc, char** argv);
     * ```
     *
     * @tparam ClassType The type of the class containing the member function.
     * @param method A pointer to the member function to bind.
     * @param name The name to associate with the bound function.
     */
    template<typename ClassType>
    void bindFunction(Nebulite::Constants::ERROR_TYPE (ClassType::*method)(int, char**),const std::string& name,const std::string& help) {
        // Automatically pass 'this' (the derived class instance) to bindFunction
        funcTree->bindFunction(
            static_cast<ClassType*>(this),  // Auto-cast to correct type
            method,                         // Member function pointer
            name, 
            help
        );
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
    DomainModuleWrapper(const DomainModuleWrapper&) = delete;
    DomainModuleWrapper& operator=(const DomainModuleWrapper&) = delete;

protected:
    //------------------------------------------
    // Linkages
    DomainType* domain;                 // Workspace of the DomainModule
    FuncTree<Nebulite::Constants::ERROR_TYPE>* funcTree;     // Where to bind the expanded functions
};
}   // namespace Interaction
}   // namespace Execution
}   // namespace Nebulite