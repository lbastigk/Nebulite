//------------------------------------------
// Includes

// Nebulite
#include "Data/Tiling.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Data {

std::vector<Batch> const& Tile::getBatches() const {
    return batches;
}

void Tile::clearBatches() {
    batches.clear();
    lastBatchesUpdate.update();
}

void Tile::appendBatch(Batch const& batch) {
    batches.push_back(batch);
    lastBatchesUpdate.update();
}

void Tile::moveObjects(std::vector<Core::RenderObject*>& destination) {
    for (auto& [objects, _] : batches) {
        // Move all objects to trash
        std::ranges::move(objects.begin(), objects.end(), std::back_inserter(destination));
        objects.clear(); // Remove all objects from the batch
    }
    batches.clear();
}

bool Tile::insertIfCostGoalMatches(Core::RenderObject* toAppend) {
    auto const it = std::ranges::find_if(
        batches.begin(),
        batches.end(),
        // NOLINTNEXTLINE
        [&](Batch const& b) {
            if constexpr (batchCostGoal == 0) {
                // NOLINTNEXTLINE
                return true; // No cost goal, accept all batches
            }
            else {
                // NOLINTNEXTLINE
                return b.estimatedCost <= batchCostGoal;
            }
        }
    );
    if (it != batches.end()) {
        it->push(toAppend);
        return true;
    }
    return false;
}

void Tile::update(std::vector<Core::RenderObject*>& to_move, std::vector<Core::RenderObject*>& to_delete, TilingInformation const& tilingInformation, TileCoordinate const& coord) {
    for (auto& batch : batches) {
        std::vector<Core::RenderObject*> to_move_local;
        std::vector<Core::RenderObject*> to_delete_local;

        for (auto* obj : batch.objects) {
            if ( auto const event = obj->update(); event != Constants::Event::Success) {
                Global::instance().notifyEvent(event);
            }
            if (!obj->flag.deleteFromScene) {
                if (RenderObjectContainer::getTilePos(obj->getPosition(), tilingInformation) != coord) {
                    to_move_local.push_back(obj);
                }
            } else {
                to_delete_local.push_back(obj);
            }
        }
        // All objects to move are collected in queue
        for (auto* ptr : to_move_local) {
            batch.removeObject(ptr);
        }

        // All objects to delete are collected in trash
        for (auto* ptr : to_delete_local) {
            batch.removeObject(ptr);
        }

        std::ranges::move(to_move_local.begin(), to_move_local.end(), std::back_inserter(to_move));
        std::ranges::move(to_delete_local.begin(), to_delete_local.end(), std::back_inserter(to_delete));
    }
}

} // namespace Nebulite::Data
