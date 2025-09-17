/**
 * @file Domain.hpp
 * 
 * @brief This file defines the Domain class, which serves as a base class for creating a Nebulite domain.
 */

#pragma once

//------------------------------------------

/**
 * @brief Macro to define a new Nebulite Domain class.
 */
#define NEBULITE_DOMAIN(DomainName) \
    class DomainName : public Nebulite::Interaction::Execution::Domain<DomainName>

/**
 * @brief Macro to define a new Nebulite DomainModule class.
 * 
 * @todo Insert macro into all DomainModules
 */
#define NEBULITE_DOMAIN_MODULE(DomainName,DomainModuleName) \
    class DomainModuleName : public Nebulite::Interaction::Execution::DomainModule<DomainName>

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
 * 
 * @todo Finding all similarities of existing domains and abstracting them into this class.
 * 
 * @todo Probably best to fully work the Trees into the Domain class as well.
 */
template<typename DomainType>
class Domain{
public:
    Domain(std::string domainName, DomainType* domain, Nebulite::Utility::JSON* doc)
    {
        this->domainName = domainName;
        this->domain = domain;
        this->doc = doc;
        funcTree = new Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>( 
                domainName, 
                Nebulite::Constants::ERROR_TYPE::NONE, 
                Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID
            );
    };

    /**
     * @brief Factory method for creating DomainModule instances with proper linkage
     * 
     * @tparam DomainModuleType The type of module to initialize
     */
    template<typename DomainModuleType>
    void initModule(std::string moduleName) {
        auto DomainModule = std::make_unique<DomainModuleType>(moduleName, domain, funcTree);
        modules.push_back(std::move(DomainModule));
    }

    /**
     * @brief Binds a variable to the FuncTree.
     */
    void bindVariable(std::string* varPtr, const std::string& name, const std::string& helpDescription){
        funcTree->bindVariable(varPtr, name, helpDescription);
    }

    /**
     * @brief Binds all functions from a inherited FuncTree to the main FuncTree for parsing.
     */
    void inherit(Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>* inheritedFuncTree){
        funcTree->inherit(inheritedFuncTree);
    }

    //------------------------------------------
    // To overwrite

    /**
     * @brief Updates the domain.
     */
    virtual void update(){
        // We cannot directly access the potential subdomain JSON here,
        // so this function needs to be overwritten in the derived class.
    }

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
    Nebulite::Utility::JSON* getDoc(){return doc;};

    //Nebulite::Interaction::Execution::FuncTree* getFuncTree(){return &funcTree;};

    //------------------------------------------
    // Command parsing

    /**
	 * @brief Parses a string into a Nebulite command.
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
	 */
	Nebulite::Constants::ERROR_TYPE parseStr(const std::string& str){
        return funcTree->parseStr(str);
    }

    /**
     * @brief Necessary operations before parsing commands.
     */
    virtual Nebulite::Constants::ERROR_TYPE preParse(){
        return Nebulite::Constants::ERROR_TYPE::NONE;
    }

//protected:
    /**
     * @brief Reference to the domain itself
     */
    DomainType* domain;

    /**
     * @brief Parsing interface for domain-specific commands.
     * 
     * We use a pointer here so we can 
     * easily create the object with a inherited FuncTree inside the constructor.
     * 
     * The Tree is then shared with the DomainModules for modification.
     */
    Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>* funcTree;

    /**
     * @brief Stores all available modules
     */
    std::vector<std::unique_ptr<Nebulite::Interaction::Execution::DomainModule<DomainType>>> modules;

private:
    std::string domainName;

    /**
     * @brief Each domain uses a JSON document to store its data.
     * We use a pointer here, as the JSON class itself is a domain.
     * Meaning the internal JSON doc references to itself.
     */
    Nebulite::Utility::JSON* doc;
};
}   // namespace Execution
}   // namespace Interaction
}   // namespace Nebulite