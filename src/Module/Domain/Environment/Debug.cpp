//------------------------------------------
// Includes

// Standard library
#include <array>
#include <cstddef>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>

// Nebulite
#include "Constants/Event.hpp"
#include "Core/Environment.hpp"
#include "Data/Batch.hpp"
#include "Data/Tiling.hpp"
#include "Module/Domain/Environment/Debug.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Environment {

Constants::Event Debug::updateHook() {
    std::size_t containerTotalTiles = 0;
    std::size_t containerTotalCost = 0;
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
    std::array<std::size_t, Core::Environment::layerCount> countPerLayer{};

    // Get object count per active tile
    domain.containerIteration([&](Data::TileCoordinate const& tileCoordinate, Core::Environment::Layer const layer, Data::Tile const& tile) {
        // Storing the information in a matrix [x][y] is not possible, as the tile positions can be negative
        // Instead, we store the position as key x<tile.x>y<tile.y>
        std::string const tileName = "x" + std::to_string(tileCoordinate.x) + "y" + std::to_string(tileCoordinate.y);
        auto const layerKey = Key::containerObjectCount.addMember("layer").addIndex(static_cast<size_t>(layer));
        auto const tileKey = layerKey.addMember("tile").addMember(tileName);
        auto const& batches = tile.getBatches();
        auto const tileObjectCount = std::accumulate(batches.begin(), batches.end(), std::size_t{0}, [](std::size_t const acc, Data::Batch const& batch) {
            return acc + batch.objects.size();
        });
        moduleScope.set<size_t>(tileKey, tileObjectCount);
        countPerLayer[static_cast<std::size_t>(layer)] += tileObjectCount;
    });

    // Set total count per layer and accumulate
    std::size_t objectCount = 0;
    for (auto const [layer, count] : countPerLayer | std::views::enumerate) {
        auto const layerKey = Key::containerObjectCount.addMember("layer").addIndex(static_cast<size_t>(layer));
        moduleScope.set<size_t>(layerKey.addMember("total"), count);
        objectCount += count;
    }

    // Set total count
    moduleScope.set<size_t>(Key::containerObjectCount.addMember("total"), objectCount);
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::Environment
