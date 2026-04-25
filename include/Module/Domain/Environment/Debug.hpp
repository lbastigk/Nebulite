/**
 * @file Debug.hpp
 * @brief Contains the Debug DomainModule for the Environment domain.
 */

#ifndef NEBULITE_DOMAINMODULE_ENVIRONMENT_DEBUG
#define NEBULITE_DOMAINMODULE_ENVIRONMENT_DEBUG

//------------------------------------------
// Includes

// Nebulite
#include "Constants/StandardCapture.hpp"
#include "Constants/KeyNames.hpp"
#include "Data/Document/JsonScope.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Environment;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::Environment {
/**
 * @class Nebulite::Module::Domain::Environment::Debug
 * @brief DomainModule for debugging utilities within the Environment domain.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Environment, Debug) {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    //------------------------------------------
    // Keys in the global document

    struct Key : Data::KeyGroup<"renderer.environment.debug."> {
        // Container info
        static auto constexpr containerTotalTiles = makeScoped("container.totalTiles");
        static auto constexpr containerTotalCost = makeScoped("container.totalCost");
    };


    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Environment, Debug) {}
};
} // namespace Nebulite::Module::Domain::Environment
#endif // NEBULITE_DOMAINMODULE_ENVIRONMENT_DEBUG
