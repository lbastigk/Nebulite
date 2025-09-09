/**
 * @file Domain.hpp
 * 
 * @brief This file defines the Domain class, which serves as a base class for creating a Nebulite domain.
 */

#pragma once

//------------------------------------------
// Includes
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/FuncTree.hpp"


//------------------------------------------
// Forward declarations

namespace Nebulite{
    namespace Utility{
        /**
         * @brief Forward declaration of JSON class. 
         * We cannot include JSON directly as JSON itself is a domain (circular dependency)
         */
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
 * @todo Finding all similarities of existing domains and abstracting them into this class.
 */
class Domain{
public:
    Domain(std::string domainName, FuncTree<Nebulite::Constants::ERROR_TYPE>* subtree ,Nebulite::Utility::JSON* doc) : doc(doc){
        funcTree = std::make_shared<
            Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>>( 
                domainName, 
                Nebulite::Constants::ERROR_TYPE::NONE, 
                Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID, 
                subtree
            );
    };

    //------------------------------------------
    // Basic setters and getters

    /**
	 * @brief Sets a value in the Domain's JSON document.
	 * 
	 * @tparam T The type of the value to set.
	 * @param key The key of the value to set.
	 * @param data The value to set.
	 */
    template <typename T> void valueSet(const char* key, const T data){
        doc->set(key,data);
    }

    /**
     * @brief Gets a value from the Domain's JSON document.
     * 
     * @tparam T The type of the value to get.
     * @param key The key of the value to get.
     * @param defaultValue The default value to return if the key is not found.
     * @return The value from the JSON document, or the default value if not found.
     */
    template <typename T> T valueGet(const char* key, const T& defaultValue = T()){
        return doc->get<T>(key,defaultValue);
    }

    /**
     * @brief Gets a pointer to the internal JSON document of the domain.
     * 
     * Each domain uses a JSON document to store its data.
     * 
     * @return A pointer to the internal JSON document.
     */
    Nebulite::Utility::JSON* getDoc();

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
	virtual Nebulite::Constants::ERROR_TYPE parseStr(const std::string& str);

    /**
     * @brief Updates the domain.
     */
    virtual void update();

protected:
    /**
     * @brief Each domain uses a JSON document to store its data.
     * We use a pointer here, as the JSON class itself is a domain.
     * Meaning the internal JSON doc references to itself.
     */
    Nebulite::Utility::JSON* doc;

    /**
     * @brief Parsing interface for domain-specific commands.
     * 
     * We use a unique pointer here so we can 
     * easily create the object with a subtree inside the constructor.
     * 
     * The Tree is then shared with the DomainModules for modification.
     */
    std::shared_ptr<Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>> funcTree;
};
}   // namespace Execution
}   // namespace Interaction
}   // namespace Nebulite