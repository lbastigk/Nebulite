//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <cstdint> // NOLINTTHISLINE
#include <iterator>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

// Nebulite
#include "Constants/ThreadSettings.hpp"
#include "Core/RenderObject.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/KeyType.hpp"
#include "Data/RenderObjectContainer.hpp"
#include "Data/RendererProcessor.hpp"
#include "Data/Tiling.hpp"
#include "Utility/Coordination/WorkDispatcher.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Data {

//------------------------------------------
// Marshalling
std::string RenderObjectContainer::serialize() {
    //------------------------------------------
    // Setup

    // Initialize RapidJSON document
    JsonScope doc;
    auto const objectsArrayKey = doc.getRootScope().addMember("objects");

    //------------------------------------------
    // Get all objects in container
    std::size_t i = 0;
    for (auto& currentBatch : std::views::values(ObjectContainer)) {
        for (auto& [objects, _] : currentBatch) {
            for (auto const& obj : objects) {
                JSON obj_serial;
                obj_serial.deserialize(obj->serialize());

                // insert into doc
                doc.setSubDoc(objectsArrayKey.addIndex(i), obj_serial);
                i++;
            }
        }
    }

    //------------------------------------------
    // Return as string
    return doc.serialize();
}

void RenderObjectContainer::deserialize(std::string const& serialOrLink, TilingInformation const& tilingInformation, Utility::IO::Capture& capture) {
    JsonScope doc;
    auto const objectsArrayKey = doc.getRootScope().addMember("objects");
    doc.deserialize(serialOrLink);
    if (doc.memberType(objectsArrayKey) == KeyType::array) {
        for (size_t i = 0; i < doc.memberSize(objectsArrayKey); i++) {
            auto objectKey = objectsArrayKey.addIndex(i);

            // Check if serial or not:
            auto ro_serial = doc.get<std::string>(objectKey);
            std::string str;
            if (!ro_serial.has_value()) {
                JSON tmp;
                tmp = doc.getSubDoc(objectKey);
                str = tmp.serialize();
            }
            else {
                str = ro_serial.value();
            }

            auto* ro = new Core::RenderObject(capture);
            ro->deserialize(str);
            append(ro, tilingInformation);
        }
    }
}

//------------------------------------------
// Pipeline

void RenderObjectContainer::append(Core::RenderObject* toAppend, TilingInformation const& tilingInformation) {
    auto const pos = getTilePos(toAppend->getPosition(), tilingInformation);

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

void RenderObjectContainer::update(std::vector<TileCoordinate> const& tiles, TilingInformation const& tilingInformation, RendererProcessor const& rendererProcessor) {
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

    size_t workerIdx = 0;
    TileCoordinate lastPos;
    for (auto pos : tiles) {
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
            currentWorker.tilingInformation = tilingInformation;
            currentWorker.cost += batch.estimatedCost;

            // Set last position for next iteration
            lastPos = pos;
        }
    }

    // Process rest
    rendererProcessor.processPool();

    // Objects to move to new tile positions
    for (auto* const obj_ptr : reinsertionProcess.queue) {
        append(obj_ptr, tilingInformation);
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

void RenderObjectContainer::reinsertAllObjects(TilingInformation const& tilingInformation) {
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
        append(ptr, tilingInformation);
    }
}

bool RenderObjectContainer::isValidPosition(TileCoordinate const& position) const {
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
    return info;
}

TileCoordinate RenderObjectContainer::getTilePos(Core::RenderObject::Position const& pos, TilingInformation const& tilingInformation) {
    // The usage of double casting seems unnecessary here,
    // there's probably an easier way using just integer division and maybe some modulo shenanigans
    return {
        // Small addition of 0.01 is necessary for pos where pos mod tilingInfo == 0, otherwise its tile coord is wrong
        static_cast<int16_t>((pos.x+0.01) / static_cast<double>(tilingInformation.w) - (pos.x < 0 ? 1 : 0)),
        static_cast<int16_t>((pos.y+0.01) / static_cast<double>(tilingInformation.h) - (pos.y < 0 ? 1 : 0)),
    };
}

} // namespace Nebulite::Data
