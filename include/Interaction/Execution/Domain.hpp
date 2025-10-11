/**
 * @file Domain.hpp
 * 
 * @brief This file defines the Domain class, which serves as a base class for creating a Nebulite domain.
 */

#pragma once

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
namespace Nebulite{
namespace Interaction{
namespace Execution{
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
    Domain(std::string domainName, DomainType* domain, Nebulite::Utility::JSON* doc)
    : domainName(domainName), domain(domain), doc(doc)
    {
        funcTree = new Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>( 
                domainName, 
                Nebulite::Constants::ErrorTable::NONE(), 
                Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTIONCALL_INVALID()
            );
    };

    /**
     * @brief Factory method for creating DomainModule instances with proper linkage
     * 
     * @tparam DomainModuleType The type of module to initialize
     * @param moduleName The name of the module
     */
    template<typename DomainModuleType>
    void initModule(std::string moduleName) {
        auto DomainModule = std::make_unique<DomainModuleType>(moduleName, domain, funcTree);
        modules.push_back(std::move(DomainModule));
    }

    /**
     * @brief Binds a variable to the FuncTree.
     * 
     * For binding functions or subtrees, use the DomainModule interface.
     */
    void bindVariable(std::string* varPtr, const std::string& name, const std::string* helpDescription){
        funcTree->bindVariable(varPtr, name, helpDescription);
    }

    /**
     * @brief Binds all functions from a inherited FuncTree to the main FuncTree for parsing.
     */
    template<typename ToInheritFrom>
    void inherit(Domain<ToInheritFrom>* toInheritFrom){
        if(toInheritFrom != nullptr){
            funcTree->inherit(toInheritFrom->funcTree);
        }
    }

    //------------------------------------------
    // To overwrite

    /**
     * @brief Updates the domain.
     * 
     * On overwriting, make sure to update all subdomains and domainmodules as well.
     */
    virtual Nebulite::Constants::Error update(){return Nebulite::Constants::ErrorTable::NONE();};

    //------------------------------------------
    // Getting private members

    /**
     * @brief Gets a pointer to the internal JSON document of the domain.
     * 
     * Each domain uses a JSON document to store its data.
     * For the JSON domain, this is a reference to itself.
     * For others, it's a reference to their JSON document.
     * 
     * @return A pointer to the internal JSON document.
     */
    Nebulite::Utility::JSON* getDoc() const { return doc; };

    //------------------------------------------
    // Command parsing

    /**
	 * @brief Parses a string into a Nebulite command and prints potential errors to stderr.
	 * 
	 * Make sure the first arg is a name and not the function itself!
	 * 
	 * - `parseStr("set text.str Hello World")` -> does not work!
	 * 
	 * - `parseStr("<someName> set text.str Hello World")` -> works
	 * 
	 * The first argument is reserved for debugging and should be used as a way to tell the parser from where it was called:
	 * ```cpp
	 * void myFunction() {
	 *   parseStr("myFunction set text.str Hello World");
	 * }
	 * ```
	 * If set fails, we can use the first argument `argv[0]` to identify the source of the command.
	 * 
	 * @param str The string to parse.
	 * @return Potential errors that occured on command execution
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
	Nebulite::Constants::Error parseStr(const std::string& str){
        Nebulite::Constants::Error err = funcTree->parseStr(str);
        err.print();
        return err;
    }

    /**
     * @brief Necessary operations before parsing commands.
     */
    virtual Nebulite::Constants::Error preParse(){
        return Nebulite::Constants::ErrorTable::NONE();
    }

    /**
     * @brief Sets a function to call before parsing commands.
     */
    void setPreParse(std::function<Nebulite::Constants::Error()> func){
        funcTree->setPreParse(func);
    }

    /**
     * @brief Reference to the domain itself
     */
    DomainType* const domain;
    
    /**
     * @brief Updates all DomainModules.
     */
    void updateModules(){
        for(auto& module : modules){
            module->update();
        }
    }

    /**
     * @brief Gets the last parsed string.
     */
    std::string getLastParsedString() const {
        return funcTree->getLastParsedString();
    }

private:
    /**
     * @brief The name of the domain.
     */
    std::string domainName;

    /**
     * @brief Parsing interface for domain-specific commands.
     * 
     * We use a pointer here so we can 
     * easily create the object with a inherited FuncTree inside the constructor.
     * 
     * The Tree is then shared with the DomainModules for modification.
     */
    Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>* funcTree;

    /**
     * @brief Stores all available modules
     */
    std::vector<std::unique_ptr<Nebulite::Interaction::Execution::DomainModule<DomainType>>> modules;

    /**
     * @brief Each domain uses a JSON document to store its data.
     * We use a pointer here, as the JSON class itself is a domain.
     * Meaning the internal JSON doc references to itself.
     */
    Nebulite::Utility::JSON* const doc;
};
}   // namespace Execution
}   // namespace Interaction
}   // namespace Nebulite