#include "DomainModule/Environment/Debug.hpp"
#include "Core/Environment.hpp"

namespace Nebulite::DomainModule::Environment {

Constants::Error Debug::update(){
    size_t containerTotalTiles = 0;
    size_t containerTotalCost = 0;
    size_t totalWorkers = 0;
    for (auto const& layer : domain.getAllLayers()) {
        // NOLINTNEXTLINE
        auto const& info = layer.getContainerInfo();
        containerTotalTiles += info.containerTotalTiles;
        containerTotalCost += info.containerTotalCost;
        totalWorkers += info.totalWorkers;
    }
    moduleScope.set<size_t>(Key::containerTotalTiles, containerTotalTiles);
    moduleScope.set<size_t>(Key::containerTotalCost, containerTotalCost);
    moduleScope.set<size_t>(Key::workersTotalCount, totalWorkers);
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::Environment
