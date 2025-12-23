/**
 * @file Domain.hpp
 * 
 * @brief This file defines the Domain class, which serves as a base class for creating a Nebulite domain.
 *        The Domain class is split into a templated Domain class and a non-templated DomainBase class.
 *        The non-templated DomainBase class holds all common functionality for domains that do not require template parameters.
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

namespace Nebulite::Data {
// We cannot include JSON directly due to circular dependencies,
// as JSON itself is a domain.
class JSON;
}

//------------------------------------------
namespace Nebulite::Interaction::Execution {

/**
 * @class DomainBase
 * @brief Non-templated base class for all Nebulite domains.
 *        Holds all common functionality for domains that do not require template parameters.
 *        This allows for a simplified interface for accessing common domain functionality.
 */
class DomainBase {
public:
    DomainBase(std::string const& name, Data::JSON* documentPtr)
        : domainName(name), document(documentPtr),
          funcTree(std::make_shared<FuncTree<Constants::Error>>(
              name,
              Constants::ErrorTable::NONE(),
              Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTIONCALL_INVALID()
              )) {
        // Set default preParse to DomainBase::preParse
        funcTree->setPreParse([this] { return preParse(); });
    }

    virtual ~DomainBase() = default;

    //------------------------------------------
    // Disallow copying and moving

    DomainBase(DomainBase const&) = delete;
    DomainBase& operator=(DomainBase const&) = delete;
    DomainBase(DomainBase&&) = delete;
    DomainBase& operator=(DomainBase&&) = delete;

    //------------------------------------------
    // Binding, initializing and inheriting

    /**
     * @brief Binds a variable to the FuncTree.
     */
    void bindVariable(bool* varPtr, std::string const& name, std::string const* helpDescription) const {
        funcTree->bindVariable(varPtr, name, helpDescription);
    }

    /**
     * @brief Binds all functions from an inherited FuncTree to the main FuncTree for parsing.
     */
    void inherit(DomainBase const* toInheritFrom) const {
        if (toInheritFrom != nullptr) {
            funcTree->inherit(toInheritFrom->funcTree);
        }
    }

    //------------------------------------------
    // Updating

    /**
     * @brief Updates the domain.
     *        On overwriting, make sure to update all subdomains and DomainModules as well.
     */
    virtual Constants::Error update() { return Constants::ErrorTable::NONE(); }

    //------------------------------------------
    // Command parsing

    /**
     * @brief Parses a string into a Nebulite command and prints potential errors to stderr.
     * @details Make sure the first arg is a name and not the function itself!
     *          - `parseStr("set text Hello World")` -> does not work!
     *          - `parseStr("<someName> set text Hello World")` -> works
     *          The first argument is reserved for debugging and should be used as a way to tell the parser from where it was called:
     *          ```cpp
     *          void myFunction(){
     *              parseStr("myFunction set text Hello World");
     *          }
     *          ```
     *          If set fails, we can use the first argument `argv[0]` to identify the source of the command.
     *          The function is marked as `[[nodiscard]]` to ensure that the caller handles potential errors.
     *          The errors are not printed to stderr by default to allow the caller to handle them as needed.
     * @param str The string to parse.
     * @return Potential errors that occurred on command execution
     */
    [[nodiscard]] Constants::Error parseStr(std::string const& str) const {
        return funcTree->parseStr(str);
    }

    /**
     * @brief Necessary operations before parsing commands.
     * @details Override this function to implement domain-specific pre-parse logic.
     * @return Error code `Constants::ErrorTable::NONE()` if there was no critical stop,
     *         an error code otherwise.
     */
    virtual Constants::Error preParse() {
        return Constants::ErrorTable::NONE();
    }

    //------------------------------------------
    // Access to private members

    /**
     * @brief Gets a pointer to the internal JSON document of the domain.
     *        Each domain uses a JSON document to store its data.
     *        For the JSON domain, this is a reference to itself.
     *        For others, it's a reference to their JSON document.
     * @return A pointer to the internal JSON document.
     */
    [[nodiscard]] Data::JSON* getDoc() const { return document; }

    /**
     * @brief Gets the name of the domain.
     * @return The name of the domain.
     */
    [[nodiscard]] std::string const& getName() const { return domainName; }

protected:
    /**
     * @brief Offers access to the internal FuncTree for function binding.
     *        Marked as protected, as it's only used to initialize DomainModules.
     * @return A shared pointer to the internal FuncTree.
     */
    std::shared_ptr<FuncTree<Constants::Error>> getFuncTree() {
        return funcTree;
    }

private:
    /**
     * @brief The name of the domain.
     */
    std::string domainName;

    /**
     * @brief Each domain uses a JSON document to store its data.
     *        We use a pointer here, as the JSON class itself is a domain.
     *        Meaning the internal JSON doc references to itself.
     */
    Data::JSON* const document;

    /**
     * @brief Parsing interface for domain-specific commands.
     *        We use a pointer here so we can
     *        easily create the object with an inherited FuncTree inside the constructor.
     *        The Tree is then shared with the DomainModules for modification.
     */
    std::shared_ptr<FuncTree<Constants::Error>> funcTree;
};

/**
 * @class Domain
 * @brief The Domain class serves as a base class for creating a Nebulite domain.
 *        Each domain has the following features:
 *        - Setting and getting values in its internal JSON document.
 *        - Returning a pointer to its internal JSON document.
 *        - Parsing strings into Nebulite commands.
 *        - Binding additional features via DomainModules.
 *        - Updating the domain through its DomainModules.
 */
template <typename DomainType>
class Domain : public DomainBase {
private:
    /**
     * @brief Stores all available modules
     */
    std::vector<std::unique_ptr<DomainModule<DomainType>>> modules;

    //------------------------------------------
    // Inner references

    /**
     * @brief Reference to the domain itself
     *        Used to initialize DomainModules with a reference to the domain
     */
    DomainType* const domain;

public:
    Domain(std::string const& name, DomainType* domainTypePtr, Data::JSON* documentPtr)
        : DomainBase(name, documentPtr), domain(domainTypePtr) {
    }

    //------------------------------------------
    // Disallow copying and moving

    Domain(Domain const&) = delete;
    Domain& operator=(Domain const&) = delete;
    Domain(Domain&&) = delete;
    Domain& operator=(Domain&&) = delete;

    //------------------------------------------
    // Module Initialization and Updating

    /**
     * @brief Factory method for creating DomainModule instances with proper linkage
     * @tparam DomainModuleType The type of module to initialize
     * @param moduleName The name of the module
     */
    template <typename DomainModuleType>
    void initModule(std::string moduleName) {
        auto DomainModule = std::make_unique<DomainModuleType>(moduleName, domain, getFuncTree());
        DomainModule->reinit();
        modules.push_back(std::move(DomainModule));
    }

    /**
     * @brief Updates all DomainModules.
     */
    void updateModules() {
        for (auto& module : modules) {
            module->update();
        }
    }

    /**
     * @brief Re-initializes all DomainModules.
     */
    void reinitModules() {
        for (auto& module : modules) {
            module->reinit();
        }
    }
};
} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAIN_HPP
