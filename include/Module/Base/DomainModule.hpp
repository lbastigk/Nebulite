/**
 * @file DomainModule.hpp
 * @brief This file defines the DomainModule class, which extends the functionality of the FuncTree
 *        class to support category-based function bindings.
 */

#ifndef MODULE_BASE_DOMAINMODULE_HPP
#define MODULE_BASE_DOMAINMODULE_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <string>

// Nebulite
#include "Interaction/Execution/DomainTree.hpp"
#include "Module/Base/DomainModuleBase.hpp"
#include "ScopeAccessor.hpp"

//------------------------------------------
namespace Nebulite::Module::Base {
/**
 * @class Nebulite::Module::Base::DomainModule
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
        // Reference to the Domain instance this module is associated with.
        DomainType& domainReference;

        // Name of the DomainModule, useful for debugging and logging.
        std::string name;

        // JsonScope reference for this module to use as workspace.
        Data::JsonScope& scope;

        // Shared pointer to the FuncTree for binding functions and variables.
        std::shared_ptr<Interaction::Execution::DomainTree> const& funcTreePtr;

        // Const JsonScope reference for settings.
        Data::JsonScope const& settings;
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
    DomainModule& operator=(DomainModule const&) = delete;

    // Prevent moving
    DomainModule(DomainModule&&) = delete;
    DomainModule& operator=(DomainModule&&) = delete;

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
} // namespace Nebulite::Module::Base
#include "DomainModule.tpp" // NOLINT
#endif // MODULE_BASE_DOMAINMODULE_HPP
