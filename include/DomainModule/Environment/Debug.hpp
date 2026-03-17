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
#include "Data/Document/JsonScope.hpp"
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
    [[nodiscard]] Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    //------------------------------------------
    // Keys in the global document

    struct Key : Data::KeyGroup<"renderer.environment.debug."> {
        // Container info
        static auto constexpr containerTotalTiles = makeScoped("container.totalTiles");
        static auto constexpr containerTotalCost = makeScoped("container.totalCost");

        // Worker info
        static auto constexpr workersTotalCount = makeScoped("workers.total.count");
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
