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
    class DomainModuleName : public ::Nebulite::Interaction::Execution::DomainModule<DomainName>

#define NEBULITE_DOMAINMODULE_CONSTRUCTOR(DomainName,DomainModuleName) \
    DomainModuleName(std::string const& moduleName, DomainName* domain, std::shared_ptr<Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>> funcTreePtr, Nebulite::Core::GlobalSpace* globalSpace) \
    : DomainModule(moduleName, domain, funcTreePtr, globalSpace)

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/FuncTree.hpp"
#include "Utility/Capture.hpp" // Allowing logging from DomainModules

//------------------------------------------
// Pre-declarations
namespace Nebulite::Core{
    class GlobalSpace;
}

//------------------------------------------
namespace Nebulite::Interaction::Execution{
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
    DomainModule(
        std::string const& moduleName, 
        DomainType* domain, 
        std::shared_ptr<Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>> funcTreePtr, 
        Nebulite::Core::GlobalSpace* globalSpace
    )
        : moduleName(moduleName), domain(domain), global(globalSpace), funcTree(funcTreePtr){}

    /**
     * @brief Virtual destructor for DomainModule.
     */
    virtual ~DomainModule() = default;

    /**
     * @brief Virtual update function to be Overwridden by derived classes.
     */
    virtual Nebulite::Constants::Error update(){ return Nebulite::Constants::ErrorTable::NONE(); }

    /**
     * @brief Binds a member function to the FuncTree.
     * 
     * This function template allows for binding member functions of any class type
     * to the FuncTree, automatically handling the necessary type conversions.
     * 
     * Make sure the function has the signature:
     * ```cpp
     * Error functionName(int argc, char* argv[]);
     * ```
     *
     * @tparam ClassType The type of the class containing the member function.
     * @param method A pointer to the member function to bind.
     * @param name The name to associate with the bound function.
     */
    template<typename ClassType>
    void bindFunction(Nebulite::Constants::Error (ClassType::*method)(int, char**), std::string const& name, std::string const* helpDescription){
        funcTree->bindFunction(
            static_cast<ClassType*>(this),
            std::variant<
                Nebulite::Constants::Error (ClassType::*)(int, char**),
                Nebulite::Constants::Error (ClassType::*)(int, char const**)
            >(method),
            name,
            helpDescription
        );
    }

    // Overload for char const** version
    template<typename ClassType>
    void bindFunction(Nebulite::Constants::Error (ClassType::*method)(int, char const**), std::string const& name, std::string const* helpDescription){
        funcTree->bindFunction(
            static_cast<ClassType*>(this),
            std::variant<
                Nebulite::Constants::Error (ClassType::*)(int, char**),
                Nebulite::Constants::Error (ClassType::*)(int, char const**)
            >(method),
            name,
            helpDescription
        );
    }

    /**
     * @brief Binds a category to the FuncTree.
     * 
     * A category acts as a "function bundler" to the main tree.
     * 
     * @param name Name of the category
     * @param description Description of the category, shown in the help command. First line is shown in the general help, full description in detailed help
     * @return true if the category was created successfully, false if a category with the same name already exists
     */
    bool bindCategory(std::string const& name, std::string const* helpDescription){
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
    void bindVariable(bool* variablePtr, std::string const& name, std::string const* helpDescription){
        // Bind a variable to the FuncTree
        funcTree->bindVariable(variablePtr, name, helpDescription);
    }

    /**
     * @brief Log to the Nebulite logging system.
     * 
     * This function logs a message to the Nebulite logging system.
     * 
     * @param message The message to log.
     */
    void log(std::string const& message){
        Nebulite::Utility::Capture::cout() << message;
    }

    /**
     * @brief Logs to the Nebulite logging system with a newline.
     * 
     * This function logs a message to the Nebulite logging system and appends a newline.
     * 
     * @param message The message to log.
     */
    void logln(std::string const& message){
        Nebulite::Utility::Capture::cout() << message << Nebulite::Utility::Capture::endl;
    }

    /**
     * @brief Log an error to the Nebulite logging system.
     * 
     * @param message The error message to log.
     */
    void logError(std::string const& message){
        Nebulite::Utility::Capture::cerr() << message;
    }

    /**
     * @brief Logs an error to the Nebulite logging system with a newline.
     * 
     * @param message The error message to log.
     */
    void logErrorln(std::string const& message){
        Nebulite::Utility::Capture::cerr() << message << Nebulite::Utility::Capture::endl;
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
    std::shared_ptr<Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>> funcTree;
};
}   // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_HPP