//------------------------------------------
// Includes

// Nebulite
#include "Core/Environment.hpp"
#include "Module/Domain/Environment/Debug.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Environment {

Constants::Event Debug::updateHook() {
    size_t containerTotalTiles = 0;
    size_t containerTotalCost = 0;
    for (auto const& layer : domain.getAllLayers()) {
        // NOLINTNEXTLINE
        auto const& info = layer.getContainerInfo();
        containerTotalTiles += info.containerTotalTiles;
        containerTotalCost += info.containerTotalCost;
    }
    moduleScope.set<size_t>(Key::containerTotalTiles, containerTotalTiles);
    moduleScope.set<size_t>(Key::containerTotalCost, containerTotalCost);
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::Environment
