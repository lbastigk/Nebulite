//------------------------------------------
// Includes

// Nebulite
#include "Core/RenderObject.hpp"
#include "Data/RendererProcessor.hpp"
#include "Data/RenderObjectContainer.hpp"
#include "Data/Document/JSON.hpp"
#include "Utility/Coordination/WorkDispatcher.hpp"

//------------------------------------------
namespace Nebulite::Data {

//------------------------------------------
// Marshalling
std::string RenderObjectContainer::serialize() {
    //------------------------------------------
    // Setup

    // Initialize RapidJSON document
    JSON doc;

    //------------------------------------------
    // Get all objects in container
    int i = 0;
    for (auto& currentBatch : std::views::values(ObjectContainer)) {
        for (auto& [objects, _] : currentBatch) {
            for (auto const& obj : objects) {
                JSON obj_serial;
                obj_serial.deserialize(obj->serialize());

                // insert into doc
                std::string key = "objects[" + std::to_string(i) + "]";
                doc.setSubDoc(key.c_str(), obj_serial);
                i++;
            }
        }
    }

    //------------------------------------------
    // Return as string
    return doc.serialize();
}

void RenderObjectContainer::deserialize(std::string const& serialOrLink, uint16_t const& dispResX, uint16_t const& dispResY, Utility::Capture& capture) {
    JSON layer;
    layer.deserialize(serialOrLink);
    if (layer.memberType("objects") == KeyType::array) {
        for (uint32_t i = 0; i < layer.memberSize("objects"); i++) {
            std::string key = "objects[" + std::to_string(i) + "]";

            // Check if serial or not:
            auto ro_serial = layer.get<std::string>(key);
            std::string str;
            if (ro_serial.error()) {
                JSON tmp;
                tmp = layer.getSubDoc(key);
                str = tmp.serialize();
            }
            else {
                str = ro_serial.value();
            }

            auto* ro = new Core::RenderObject(capture);
            ro->deserialize(str);
            append(ro, dispResX, dispResY);
        }
    }
}

//------------------------------------------
// Pipeline

void RenderObjectContainer::append(Core::RenderObject* toAppend, uint16_t const& dispResX, uint16_t const& dispResY) {
    std::pair<int16_t, int16_t> const pos = getTilePos(toAppend, dispResX, dispResY);

    // Try to insert into an existing batch
    auto const it = std::ranges::find_if(
        ObjectContainer[pos].begin(),
        ObjectContainer[pos].end(),
        // NOLINTNEXTLINE
        [](Batch const& b) {
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
    if (it != ObjectContainer[pos].end()) {
        it->push(toAppend);
        return;
    }

    // No existing batch could accept the object, so create a new one
    Batch newBatch;
    newBatch.push(toAppend);
    ObjectContainer[pos].push_back(std::move(newBatch));
}

void RenderObjectContainer::update(int16_t const& tilePosX, int16_t const& tilePosY, uint16_t const& dispResX, uint16_t const& dispResY, RendererProcessor const& rendererProcessor) {
    //------------------------------------------
    // Define tile offsets that are being rendered

    // Currently, tile size is based on resolution so we render a 3x3 grid of tiles
    static std::vector<int16_t> const tileOffsetsX = {-1, 0, 1};
    static std::vector<int16_t> const tileOffsetsY = {-1, 0, 1};

    //------------------------------------------
    // 2-Step Deletion

    // Deletion flag --> Trash --> Purgatory --> Destructor
    // This way, any invokes previously send are safe to never access any deleted memory

    // Finalize deletion of objects in purgatory
    if (!deletionProcess.purgatory.empty()) {
        for (auto const& ptr : deletionProcess.purgatory) {
            delete ptr;
        }
        deletionProcess.purgatory.clear();
    }

    // Move trash into purgatory
    deletionProcess.purgatory.swap(deletionProcess.trash);

    //------------------------------------------
    // Update only tiles that might be visible

    // since one tile is size of screen, a max of 9 tiles
    // [P] - Tile with Player
    // [#] - loaded Tiles
    // [ ] - inactive Tiles
    //
    // [ ][ ][ ][ ][ ][ ][ ][ ][ ]
    // [ ][ ][ ][ ][ ][ ][ ][ ][ ]
    // [ ][ ][ ][ ][ ][ ][ ][ ][ ]
    // [ ][ ][ ][#][#][#][ ][ ][ ]
    // [ ][ ][ ][#][P][#][ ][ ][ ]
    // [ ][ ][ ][#][#][#][ ][ ][ ]
    // [ ][ ][ ][ ][ ][ ][ ][ ][ ]
    // [ ][ ][ ][ ][ ][ ][ ][ ][ ]
    // [ ][ ][ ][ ][ ][ ][ ][ ][ ]
    //
    // NOTE:
    // Later on it may be better to use a fixed tile size and load enough to cover the screen, + maybe 1-2 tiles extra on each side.
    // Or, perhaps even better, use a fixed tile size + a fixed loading radius around the player position.
    // This, however, requires the renderer to determine a maximum resolution beforehand based on the radius.
    // Meaning it has to discard any requested resolution that is too high for the radius.
    // Note that we cannot directly use the maximum tile radius, as for some positions it may be smaller!
    // So we should subtract at least one tile, perhaps even two to be safe.
    // Or we go the actual good way and do the math to determine hMax/wMax based on the radius and tile size.

    // Create worker threads for batches in visible tiles, based on batch cost and other factors
    size_t workerIdx = 0;
    std::pair<int16_t, int16_t> lastPos;
    for (int16_t const dX : tileOffsetsX) {
        for (int16_t const dY : tileOffsetsY) {
            // Current tile position we want to update
            std::pair<int16_t, int16_t> pos = std::make_pair(tilePosX - dX, tilePosY - dY);

            // Check if container has tile at position, if not, skip
            auto const it = ObjectContainer.find(pos);
            if (it == ObjectContainer.end()) {
                continue;
            }

            // Create worker threads that try to closely match the batch cost goal
            // If the current batch added to the worker exceeds the batch cost goal, we start a new worker thread for the next batch
            for (auto& batch : it->second) {
                if (rendererProcessor.batchWorkerPool[workerIdx]->workspace.cost + batch.estimatedCost > batchCostGoal || lastPos != pos) {
                    workerIdx++;
                }

                if (workerIdx >= Constants::ThreadSettings::getRendererWorkerCount()) {
                    rendererProcessor.processPool(); // Process all workers and reset pool
                    workerIdx = 0; // Reset worker count for new batch of work
                }

                // Get current worker, set up workspace and add work to it
                auto& currentWorker = rendererProcessor.batchWorkerPool[workerIdx]->workspace;
                currentWorker.work.push_back(&batch);
                currentWorker.pos = pos;
                currentWorker.dispResX = dispResX;
                currentWorker.dispResY = dispResY;
                currentWorker.cost += batch.estimatedCost;

                // Set last position for next iteration
                lastPos = pos;
            }
        }
    }

    // Process rest
    rendererProcessor.processPool();

    // Objects to move to new tile positions
    for (auto const obj_ptr : reinsertionProcess.queue) {
        append(obj_ptr, dispResX, dispResY);
    }
    reinsertionProcess.queue.clear();
}

Core::RenderObject* RenderObjectContainer::getObjectFromId(size_t const& domainId) {
    // Go through all batches
    for (auto& batches : std::views::values(ObjectContainer)) {
        for (auto& [objects, _] : batches) {
            for (auto const& object : objects) {
                if (object->getId() == domainId) {
                    return object;
                }
            }
        }
    }
    return nullptr; // Not found
}

void RenderObjectContainer::reinsertAllObjects(uint16_t const& dispResX, uint16_t const& dispResY) {
    // Collect all objects
    std::vector<Core::RenderObject*> toReinsert;
    for (auto& batches : std::views::values(ObjectContainer)) {
        for (auto& [objects, _] : batches) {
            // Collect all objects from the batch
            std::ranges::copy(objects.begin(), objects.end(), std::back_inserter(toReinsert));
        }
    }

    // Fully reset container
    ObjectContainer.clear();

    // Reinsert
    for (auto const& ptr : toReinsert) {
        append(ptr, dispResX, dispResY);
    }
}

bool RenderObjectContainer::isValidPosition(std::pair<uint16_t, uint16_t> const& position) {
    // Check if ObjectContainer is not empty
    auto const it = ObjectContainer.find(position);
    return it != ObjectContainer.end();
}

void RenderObjectContainer::purgeObjects() {
    for (auto it = ObjectContainer.begin(); it != ObjectContainer.end();) {
        for (auto& [objects, _] : it->second) {
            // Move all objects to trash
            std::ranges::move(objects.begin(), objects.end(), std::back_inserter(deletionProcess.trash));
            objects.clear(); // Remove all objects from the batch
        }
        ++it;
    }
}

size_t RenderObjectContainer::getObjectCount() const {
    // Calculate the total item count
    size_t totalCount = 0;
    for (auto const it = ObjectContainer.begin(); it != ObjectContainer.end();) {
        totalCount += it->second.size();
    }
    return totalCount;
}

RenderObjectContainer::ContainerInfo RenderObjectContainer::getContainerInfo() const {
    ContainerInfo info;

    // Container stats
    info.containerTotalTiles = ObjectContainer.size();
    info.containerTotalCost = 0;
    for (auto const& batches : std::views::values(ObjectContainer)) {
        for (auto const& [_, cost] : batches) {
            info.containerTotalCost += cost;
        }
    }

    // Worker stats
    info.totalWorkers = Constants::ThreadSettings::getInvokeWorkerCount();

    return info;
}

std::pair<int16_t, int16_t> RenderObjectContainer::getTilePos(Core::RenderObject const* toAppend, uint16_t const& displayResolutionX, uint16_t const& displayResolutionY) {
    auto [x, y] = toAppend->getPosition();
    auto correspondingTilePositionX = static_cast<int16_t>(x / static_cast<double>(displayResolutionX));
    auto correspondingTilePositionY = static_cast<int16_t>(y / static_cast<double>(displayResolutionY));
    return std::make_pair(correspondingTilePositionX, correspondingTilePositionY);
}

} // namespace Nebulite::Core
