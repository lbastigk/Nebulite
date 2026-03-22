#include "DomainModule/Environment/Debug.hpp"
#include "Core/Environment.hpp"

namespace Nebulite::DomainModule::Environment {

Constants::Event Debug::update(){
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

} // namespace Nebulite::DomainModule::Environment
