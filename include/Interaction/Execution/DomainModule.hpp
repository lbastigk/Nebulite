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
    struct ConstructorParams {
        DomainType& domainReference; // Reference to the Domain instance this module is associated with.
        std::string name; // Name of the DomainModule, useful for debugging and logging.
        Data::JsonScope& scope; // JsonScope reference for this module to use as workspace.
        std::shared_ptr<DomainTree> const& funcTreePtr; // Shared pointer to the FuncTree for binding functions and variables.
        Data::JsonScope const& settings; // Const JsonScope reference for settings.
    };

    /**
     * @brief Constructor for the DomainModule base class.
     * @details The constructor initializes the DomainModule with a reference to the domain and
     *          the FuncTree.
     * @param params Struct containing all necessary parameters for constructing the DomainModule.
     *               Kept in a struct to keep the parameter list of derived DomainModules short
     */
    explicit DomainModule(ConstructorParams const& params);

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
#include "Interaction/Execution/DomainModule.tpp"
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAIN_MODULE_HPP
