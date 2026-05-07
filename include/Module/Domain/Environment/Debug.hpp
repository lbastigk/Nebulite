/**
 * @file Debug.hpp
 * @brief Contains the Debug DomainModule for the Environment domain.
 */

#ifndef NEBULITE_MODULE_DOMAIN_ENVIRONMENT_DEBUG
#define NEBULITE_MODULE_DOMAIN_ENVIRONMENT_DEBUG

//------------------------------------------
// Includes

// Nebulite
#include "Constants/KeyNames.hpp"
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
class Debug final : public Interaction::Execution::DomainModule<Core::Environment> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    // TODO: Add function to dump object count per tile

    Constants::Event fetchContainer() const ;
    static auto constexpr fetchContainerName = "fetch-container";
    static auto constexpr fetchContainerDesc = "Fetches and returns information about the container, including object count per tile.";

    //------------------------------------------
    // Keys in the global document

    struct Key : Data::KeyGroup<"renderer.environment.debug."> {
        // Container info
        static auto constexpr containerTotalTiles = makeScoped("container.totalTiles");
        static auto constexpr containerTotalCost = makeScoped("container.totalCost");
        static auto constexpr containerObjectCount = makeScoped("container.objectCount");
    };

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit Debug(ConstructorParams const& params) : DomainModule(params) {
        bindFunction(&Debug::fetchContainer, fetchContainerName, fetchContainerDesc);
    }
};
} // namespace Nebulite::Module::Domain::Environment
#endif // NEBULITE_MODULE_DOMAIN_ENVIRONMENT_DEBUG
