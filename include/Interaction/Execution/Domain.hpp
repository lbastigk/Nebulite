/**
 * @file Domain.hpp
 * 
 * @brief This file defines the Domain class, which serves as a base class for creating a Nebulite domain.
 *
 *
 * @todo Implement a DomainBase class without templates to reduce code bloat.
 *       This allows us to access common functionality without needing to instantiate templates.
 *       E.g. for context structs in the Invoke class: DomainBase* self, DomainBase* other, etc...
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
// Forward declarations

namespace Nebulite{
    namespace Core{
        // we cannot include GlobalSpace directly due to circular dependencies,
        // as GlobalSpace itself is a Domain.
        class GlobalSpace;
    } 
    /**
     * @todo Is this necessary?
     */
    namespace Interaction{
        class Invoke;
    }
    namespace Utility{
        // We cannot include JSON directly due to circular dependencies,
        // as JSON itself is a domain.
        class JSON;
    }
}



//------------------------------------------
namespace Nebulite::Interaction::Execution{
/**
 * @class Domain
 * @brief The Domain class serves as a base class for creating a Nebulite domain.
 * 
 * Each domain has the following features:
 * 
 * - Setting and getting values in its internal JSON document.
 * - Returning a pointer to its internal JSON document.
 * - Parsing strings into Nebulite commands.
 * - Binding additional features via DomainModules.
 * - Updating the domain through its DomainModules.
 */
template<typename DomainType>
class Domain{
    template<typename> friend class Domain;  // All Domain<T> instantiations are friends, so we can access each other's private members
public:
    Domain(std::string domainName, DomainType* domain, Utility::JSON* doc, Core::GlobalSpace* global)
    : domainName(domainName),
      funcTree(std::make_shared<FuncTree<Constants::Error>>(
          domainName, 
          Constants::ErrorTable::NONE(), 
          Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTIONCALL_INVALID()
      )),
      domain(domain), doc(doc), global(global)
    {}

    virtual ~Domain() = default;
    
    //------------------------------------------
    // Binding, initializing and inheriting

    /**
     * @brief Factory method for creating DomainModule instances with proper linkage
     * 
     * @tparam DomainModuleType The type of module to initialize
     * @param moduleName The name of the module
     */
    template<typename DomainModuleType>
    void initModule(std::string moduleName){
        auto DomainModule = std::make_unique<DomainModuleType>(moduleName, domain, funcTree, global);
        modules.push_back(std::move(DomainModule));
    }

    /**
     * @brief Binds a variable to the FuncTree.
     * 
     * For binding functions or categories, use the DomainModule interface.
     */
    void bindVariable(bool* varPtr, std::string const& name, std::string const* helpDescription) const {
        funcTree->bindVariable(varPtr, name, helpDescription);
    }

    /**
     * @brief Binds all functions from an inherited FuncTree to the main FuncTree for parsing.
     */
    template<typename ToInheritFrom>
    void inherit(Domain<ToInheritFrom>* toInheritFrom){
        if(toInheritFrom != nullptr){
            funcTree->inherit(toInheritFrom->funcTree);
        }
    }

    //------------------------------------------
    // Updating

    /**
     * @brief Updates the domain.
     * 
     * On overwriting, make sure to update all subdomains and DomainModules as well.
     */
    virtual Constants::Error update(){ return Constants::ErrorTable::NONE(); }

    /**
     * @brief Updates all DomainModules.
     */
    void updateModules(){
        for(auto& module : modules){
            module->update();
        }
    }

    //------------------------------------------
    // Command parsing

    /**
	 * @brief Parses a string into a Nebulite command and prints potential errors to stderr.
	 * 
	 * Make sure the first arg is a name and not the function itself!
	 * 
	 * - `parseStr("set text Hello World")` -> does not work!
	 * 
	 * - `parseStr("<someName> set text Hello World")` -> works
	 * 
	 * The first argument is reserved for debugging and should be used as a way to tell the parser from where it was called:
	 * ```cpp
	 * void myFunction(){
	 *   parseStr("myFunction set text Hello World");
	 * }
	 * ```
	 * If set fails, we can use the first argument `argv[0]` to identify the source of the command.
	 * 
	 * @param str The string to parse.
	 * @return Potential errors that occurred on command execution
     * 
     * @todo Disable printing errors if the domain is inside another domain without inheritance.
     * This needs to be done, otherwise each error is printed twice:
     * - Parent domain gets parse string, redirects parts to child
     * - child parses string, fails, prints error
     * - child returns error to parent
     * - parent prints error again
     * Idea: Add a flag to either constructor or a new method to tell each domain what kind of error handling to do.
     * Another idea would be to not print in parse, but escalate as far as possible, and only print in main.
     * 
     * To better understand the difference:
     * - If we inherit from another domain, we parse once, no double printing
     * - If we add a domain, for instance, in a DomainModule, we have a parse within a parse, leading to double printing
	 */
	[[nodiscard]] Constants::Error parseStr(std::string const& str) const {
        Constants::Error const err = funcTree->parseStr(str);
        //err.print();  // Disabled for now, but needs proper treatment later
        return err;
    }

    /**
     * @brief Necessary operations before parsing commands.
     */
    virtual Constants::Error preParse(){
        return Constants::ErrorTable::NONE();
    }

    /**
     * @brief Sets a function to call before parsing commands.
     */
    void setPreParse(std::function<Constants::Error()> const& func) const {
        funcTree->setPreParse(func);
    }

    //------------------------------------------
    // Access to private members

    /**
     * @brief Gets a pointer to the internal JSON document of the domain.
     * 
     * Each domain uses a JSON document to store its data.
     * For the JSON domain, this is a reference to itself.
     * For others, it's a reference to their JSON document.
     * 
     * @return A pointer to the internal JSON document.
     */
    [[nodiscard]] Utility::JSON* getDoc() const {return doc;}

    /**
     * @brief Gets a pointer to the globalspace.
     * 
     * @return A pointer to the globalspace.
     */
    [[nodiscard]] Core::GlobalSpace* getGlobalSpace() const {return global;}

    //------------------------------------------
    // Logging

    /**
     * @brief Logs to the Nebulite logging system with a newline.
     * 
     * This function logs a message to the Nebulite logging system and appends a newline.
     * 
     * @param message The message to log.
     */
    static void logln(std::string const& message){
        Utility::Capture::cout() << message << Utility::Capture::endl;
    }

    /**
     * @brief Log an error to the Nebulite logging system.
     * 
     * @param message The error message to log.
     */
    static void logError(std::string const& message){
        Utility::Capture::cerr() << message;
    }

    /**
     * @brief Logs an error to the Nebulite logging system with a newline.
     * 
     * @param message The error message to log.
     */
    static void logErrorln(std::string const& message){
        Utility::Capture::cerr() << message << Utility::Capture::endl;
    }

private:
    //------------------------------------------
    // Core members
    
    /**
     * @brief The name of the domain.
     */
    std::string domainName;

    //------------------------------------------
    // Modules and the FuncTree they act upon

    /**
     * @brief Stores all available modules
     */
    std::vector<std::unique_ptr<DomainModule<DomainType>>> modules;

    /**
     * @brief Parsing interface for domain-specific commands.
     * 
     * We use a pointer here so we can 
     * easily create the object with an inherited FuncTree inside the constructor.
     * 
     * The Tree is then shared with the DomainModules for modification.
     */
    std::shared_ptr<FuncTree<Constants::Error>> funcTree;

    //------------------------------------------
    // Inner references

    /**
     * @brief Reference to the domain itself
     * 
     * Used to initialize DomainModules with a reference to the domain.
     */
    DomainType* const domain;

    /**
     * @brief Each domain uses a JSON document to store its data.
     * We use a pointer here, as the JSON class itself is a domain.
     * Meaning the internal JSON doc references to itself.
     */
    Utility::JSON* const doc;

    /**
     * @brief Pointer to the globalspace, for accessing global resources and management functions.
     */
    Core::GlobalSpace* const global;
};
}   // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAIN_HPP