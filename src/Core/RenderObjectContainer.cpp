#include "Core/RenderObjectContainer.hpp"

namespace Nebulite::Core {



//------------------------------------------
//Marshalling
std::string RenderObjectContainer::serialize() {
    //------------------------------------------
    // Setup

    // Initialize RapidJSON document
    Data::JSON doc;

    //------------------------------------------
    // Get all objects in container
    int i = 0;
    for (auto& currentBatch : std::views::values(ObjectContainer)) {
        for (auto& [objects, _] : currentBatch) {
            for (auto const& obj : objects) {
                Data::JSON obj_serial;
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

void RenderObjectContainer::deserialize(std::string const& serialOrLink, uint16_t const& dispResX, uint16_t const& dispResY) {
    Data::JSON layer;
    layer.deserialize(serialOrLink);
    if (layer.memberType("objects") == Data::JSON::KeyType::array) {
        for (uint32_t i = 0; i < layer.memberSize("objects"); i++) {
            std::string key = "objects[" + std::to_string(i) + "]";

            // Check if serial or not:
            auto ro_serial = layer.get<std::string>(key);
            if (ro_serial == "{Object}") {
                Data::JSON tmp;
                tmp = layer.getSubDoc(key);
                ro_serial = tmp.serialize();
            }

            auto* ro = new RenderObject;
            ro->deserialize(ro_serial);
            append(ro, dispResX, dispResY);
        }
    }
}

//------------------------------------------
// Pipeline

namespace {
std::pair<int16_t, int16_t> getTilePos(RenderObject* toAppend, uint16_t const& displayResolutionX, uint16_t const& displayResolutionY) {
    // Calculate correspondingTilePositionX using positionX
    auto const positionX = toAppend->getDoc()->get<double>(Nebulite::Constants::KeyNames::RenderObject::positionX, 0.0);
    auto correspondingTilePositionX = static_cast<int16_t>(positionX / static_cast<double>(displayResolutionX));

    // Calculate correspondingTilePositionY using positionY
    auto const positionY = toAppend->getDoc()->get<double>(Nebulite::Constants::KeyNames::RenderObject::positionY, 0.0);
    auto correspondingTilePositionY = static_cast<int16_t>(positionY / static_cast<double>(displayResolutionY));

    // Form pair and return
    return std::make_pair(correspondingTilePositionX, correspondingTilePositionY);
}
} // anonymous namespace

void RenderObjectContainer::append(RenderObject* toAppend, uint16_t const& dispResX, uint16_t const& dispResY) {
    std::pair<int16_t, int16_t> const pos = getTilePos(toAppend, dispResX, dispResY);

    // Try to insert into an existing batch
    auto const it = std::ranges::find_if(ObjectContainer[pos].begin(), ObjectContainer[pos].end(),
                                         [](Batch const& b) { return b.estimatedCost <= BATCH_COST_GOAL; }
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

std::thread RenderObjectContainer::createBatchWorker(Batch& work, std::pair<int16_t, int16_t> pos, uint16_t dispResX, uint16_t dispResY) {
    return std::thread([&work, pos, this, dispResX, dispResY] {
        // Every batch worker has potential objects to move or delete
        std::vector<RenderObject*> to_move_local;
        std::vector<RenderObject*> to_delete_local;

        // We update each object and check if it needs to be moved or deleted
        for (auto obj : work.objects) {
            obj->update();

            if (!obj->flag.deleteFromScene) {
                if (getTilePos(obj, dispResX, dispResY) != pos) {
                    to_move_local.push_back(obj);
                }
            } else {
                to_delete_local.push_back(obj);
            }
        }

        // All objects to move are collected in queue
        for (auto ptr : to_move_local) {
            work.removeObject(ptr);
            std::scoped_lock lock(reinsertionProcess.reinsertMutex);
            reinsertionProcess.queue.push_back(ptr);
        }

        // All objects to delete are collected in trash
        for (auto ptr : to_delete_local) {
            work.removeObject(ptr);
            std::scoped_lock lock(deletionProcess.deleteMutex);
            deletionProcess.trash.push_back(ptr);
        }
    });
}

void RenderObjectContainer::update(int16_t const& tilePosX, int16_t const& tilePosY, uint16_t const& dispResX, uint16_t const& dispResY) {
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
    for (int16_t const dX : tileOffsetsX) {
        uint16_t const currentTilePosX = tilePosX - dX;
        for (int16_t const dY : tileOffsetsY) {
            uint16_t const currentTilePosY = tilePosY - dY;
            std::pair<uint16_t, uint16_t> pos = std::make_pair(currentTilePosX, currentTilePosY);
            auto& tile = ObjectContainer[pos];

            // Create batch workers for each batch in the tile
            std::ranges::transform(
                tile.begin(), tile.end(),
                std::back_inserter(batchWorkers),
                [&](auto& batch) { return createBatchWorker(batch, pos, dispResX, dispResY); }
                );
        }
    }

    // Wait for threads to be finished
    for (auto& batchWorker : batchWorkers) {
        if (batchWorker.joinable())
            batchWorker.join();
    }

    // Objects to move
    for (auto const obj_ptr : reinsertionProcess.queue) {
        append(obj_ptr, dispResX, dispResY);
    }
    reinsertionProcess.queue.clear();
}

void RenderObjectContainer::reinsertAllObjects(uint16_t const& dispResX, uint16_t const& dispResY) {
    // Collect all objects
    std::vector<RenderObject*> toReinsert;
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

//------------------------------------------
// Batch

RenderObject* RenderObjectContainer::Batch::pop() {
    if (objects.empty())
        return nullptr;

    RenderObject* obj = objects.back(); // Get last element
    estimatedCost -= obj->estimateComputationalCost(); // Adjust cost
    objects.pop_back(); // Remove from vector

    return obj;
}

void RenderObjectContainer::Batch::push(RenderObject* obj) {
    estimatedCost += obj->estimateComputationalCost();
    objects.push_back(obj);
}

bool RenderObjectContainer::Batch::removeObject(RenderObject* obj) {
    if (auto const it = std::ranges::find(objects.begin(), objects.end(), obj); it != objects.end()) {
        estimatedCost -= obj->estimateComputationalCost();
        objects.erase(it);
        return true;
    }
    return false;
}

} // namespace Nebulite::Core
