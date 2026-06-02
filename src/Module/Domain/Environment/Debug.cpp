//------------------------------------------
// Includes

// Standard library
// NOLINTNEXTLINE
#include <cstddef>
#include <numeric>
#include <string>
#include <vector>

// Nebulite
#include "Constants/Event.hpp"
#include "Core/Environment.hpp"
#include "Data/RendererProcessor.hpp"
#include "Data/Tiling.hpp"
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

Constants::Event Debug::fetchContainer() const {
    size_t objectCount = 0;
    domain.containerIteration([&](Data::TileCoordinate const& tileCoordinate, Core::Environment::Layer const& layer, Data::Tile const& tile) {
        // Storing the information in a matrix [x][y] is not possible, as the tile positions can be negative
        // Instead, we store the position as key x<tile.x>y<tile.y>
        std::string const tileName = "x" + std::to_string(tileCoordinate.x) + "y" + std::to_string(tileCoordinate.y);
        auto const layerKey = Key::containerObjectCount.addMember("layer").addIndex(static_cast<size_t>(layer));
        auto const tileKey = layerKey.addMember("tile").addMember(tileName);
        auto const& batches = tile.getBatches();
        auto const batchCount = std::accumulate(batches.begin(), batches.end(), size_t{0}, [](size_t const acc, Data::Batch const& batch) {
            return acc + batch.objects.size();
        });
        moduleScope.set<size_t>(tileKey, batchCount);
        objectCount += batchCount;
    });
    moduleScope.set<size_t>(Key::containerObjectCount.addMember("total"), objectCount);
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::Environment
