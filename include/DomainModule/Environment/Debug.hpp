/**
 * @file Debug.hpp
 * @brief Contains the Debug DomainModule for the Environment domain.
 */

#ifndef NEBULITE_DOMAINMODULE_ENVIRONMENT_DEBUG
#define NEBULITE_DOMAINMODULE_ENVIRONMENT_DEBUG

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Constants/KeyNames.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/JsonScopeBase.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Environment;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::Environment {
/**
 * @class Nebulite::DomainModule::Environment::Debug
 * @brief DomainModule for debugging utilities within the Environment domain.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Environment, Debug) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    //------------------------------------------
    // Keys in the global document

    struct Key {
        DECLARE_SCOPE("renderer.environment.debug.")

        // Container info
        static auto constexpr containerTotalTiles = MAKE_SCOPED("container.totalTiles");
        static auto constexpr containerTotalCost = MAKE_SCOPED("container.totalCost");

        // Worker info: Active
        static auto constexpr workersActiveCount = MAKE_SCOPED("workers.active.count");
        static auto constexpr workersActiveTotalCost = MAKE_SCOPED("workers.active.cost");

        // Worker info: Total
        static auto constexpr workersTotalCount = MAKE_SCOPED("workers.total.count");
    };


    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Environment, Debug) {}
};
} // namespace Nebulite::DomainModule::Environment
#endif // NEBULITE_DOMAINMODULE_ENVIRONMENT_DEBUG
