/**
 * @file DomainModule.hpp
 * @brief This file defines the DomainModule class, which extends the functionality of the FuncTree
 *        class to support category-based function bindings.
 */

#ifndef NEBULITE_INTERACTION_EXECUTION_DOMAIN_MODULE_HPP
#define NEBULITE_INTERACTION_EXECUTION_DOMAIN_MODULE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "ScopeAccessor.hpp"
#include "Interaction/Execution/DomainTree.hpp"
#include "Interaction/Execution/FuncTree.hpp"
#include "Data/Document/JsonScope.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Execution {
/**
 * @class Nebulite::Interaction::Execution::DomainModule
 * @brief Wrapper class for binding functions to a specific category in the FuncTree and adding separate update routines.
 * @details Allows for cleaner separation of object files for different categories
 *          and reduces boilerplate code when attaching functions to the FuncTree.
 *          DomainModules provide several functionalities:
 *          - Data inside their scope
 *          - Keys for other classes to use to retrieve data
 *          - Update routines
 *          - Functions to call via the FuncTree system
 */
template <typename DomainType>
class DomainModule : public DomainModuleBase {
public:
    /**
     * @brief Constructor for the DomainModule base class.
     * @details The constructor initializes the DomainModule with a reference to the domain and
     *          the FuncTree.
     * @param name Name of the DomainModule, useful for debugging and logging.
     * @param domainReference Reference to the Domain instance this module is associated with.
     * @param funcTreePtr Shared pointer to the FuncTree for binding functions and variables.
     * @param scope JsonScope reference for this module to use as workspace.
     * @param settings Const JsonScope reference for settings.
     */
    DomainModule(
        std::string name,
        DomainType& domainReference,
        std::shared_ptr<DomainTree> const& funcTreePtr,
        Data::JsonScope& scope,
        Data::JsonScope const& settings
    );

    ~DomainModule() override ;

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
    DomainType& domain;

    //------------------------------------------
    // Access token for Global Space access

    static ScopeAccessor::BaseAccessToken getDomainModuleAccessToken(DomainModule const& derivedModule) {
        return ScopeAccessor::DomainModuleToken(derivedModule);
    }
};
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
// Macros for DomainModule definition

// Macro for defining a new DomainModule class with the correct inheritance and template parameters
// NOLINTNEXTLINE
#define NEBULITE_DOMAINMODULE(DomainName,DomainModuleName) \
class DomainModuleName final : public Nebulite::Interaction::Execution::DomainModule<DomainName>

// Macro for easily passing the constructor parameters to the base class constructor in the DomainModule definition
// NOLINTNEXTLINE
#define NEBULITE_DOMAINMODULE_CONSTRUCTOR(DomainName,DomainModuleName) \
explicit DomainModuleName( \
std::string const& name, DomainName& domainReference, \
std::shared_ptr<Interaction::Execution::DomainTree> const& funcTreePtr, \
Data::JsonScope& w, \
Data::JsonScope const& s \
) \
: DomainModule(name, domainReference, std::move(funcTreePtr), w, s)

#include "Interaction/Execution/DomainModule.tpp"
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAIN_MODULE_HPP
