/**
 * @file RenderObjectContainer.hpp
 * @brief Contains the Nebulite::Core::RenderObjectContainer class.
 */

#ifndef NEBULITE_CORE_RENDER_OBJECT_CONTAINER_HPP
#define NEBULITE_CORE_RENDER_OBJECT_CONTAINER_HPP

//------------------------------------------
// Includes

// Standard library
#include <thread>

// Nebulite
#include "Core/RenderObject.hpp"
#include "Data/Document/JsonScope.hpp"

//------------------------------------------
namespace Nebulite::Core {
/**
 * @class Nebulite::Core::RenderObjectContainer
 * @brief Manages a collection of RenderObject instances in a tile-based container.
 */
class RenderObjectContainer {
public:
    /**
     * @struct Nebulite::Core::RenderObjectContainer::Batch
     * @brief Represents a batch of RenderObject instances in a given tile.
     *        `Batch -> vector<RenderObject*>`
     *        Used for threading and parallel processing of render objects.
     *        Basically a custom std::vector wrapper for easier cost management.
     */
    struct Batch {
        // Collection of RenderObjects
        std::vector<RenderObject*> objects;

        // Full estimated cost of the batch
        uint64_t estimatedCost = 0;

        /**
         * @brief Pops the last RenderObject from the batch.
         * @return Pointer to the popped RenderObject, or nullptr if batch is already empty.
         */
        RenderObject* pop();

        /**
         * @brief Pushes a RenderObject into the batch.
         * @param obj Pointer to the RenderObject to push.
         */
        void push(RenderObject* obj);

        /**
         * @brief Removes a RenderObject from the batch.
         * @param obj Pointer to the RenderObject to remove.
         * @return True if the object was removed, false otherwise.
         */
        bool removeObject(RenderObject* obj);
    };

    //------------------------------------------
    //Constructor

    /**
     * @brief Constructs a new RenderObjectContainer.
     */
    RenderObjectContainer() = default;

    //------------------------------------------
    // Serialization / Deserialization

    /**
     * @brief Serializes the RenderObjectContainer to a JSON string.
     * @return JSON string representation of the container.
     */
    std::string serialize();

    /**
     * @brief Deserializes the RenderObjectContainer from a JSON string.
     * @param serialOrLink JSON string representation of the container, or link to a json/jsonc file.
     * @param dispResX Display resolution width for tile initialization.
     * @param dispResY Display resolution height for tile initialization.
     */
    void deserialize(std::string const& serialOrLink, uint16_t const& dispResX, uint16_t const& dispResY);

    //------------------------------------------
    // Pipeline

    /**
     * @brief Appends a RenderObject to the container.
     *        Places it in the appropriate tile and batches it through cost-estimation.
     * @param toAppend Pointer to the RenderObject to append.
     * @param dispResX Display resolution width for tile placement.
     * @param dispResY Display resolution height for tile placement.
     */
    void append(RenderObject* toAppend, uint16_t const& dispResX, uint16_t const& dispResY);

    /**
     * @brief Reinserts all objects into the container.
     *        Placing them in the appropriate tile and batch.
     *        Needed for re-evaluating their positions after a resize of the display.
     * @param dispResX Display resolution width for tile placement.
     * @param dispResY Display resolution height for tile placement.
     */
    void reinsertAllObjects(uint16_t const& dispResX, uint16_t const& dispResY);

    /**
     * @brief Checks if the given tile position is valid; contains objects.
     * @param position The tile position to check: (x, y).
     * @return True if the position contains objects, false otherwise.
     */
    bool isValidPosition(std::pair<uint16_t, uint16_t> const& position);

    // removes all objects
    /**
     * @brief Moves all objects into the deletion process.
     *        It takes 2 updates to fully delete them.
     *        First they are moved to trash, then on update to purgatory,
     *        then on next update deleted.
     */
    void purgeObjects();

    /**
     * @brief Gets the total count of RenderObject instances in the container.
     * @return The total number of RenderObject instances.
     */
    [[nodiscard]] size_t getObjectCount() const;

    /**
     * @brief Responsible for updating the state of all RenderObject instances
     *        that are currently within the specified tile viewport. It takes into account the
     *        display resolution for potential re-insertions.
     * @param tilePosX The middle tile to in x-axis.
     * @param tilePosY The middle tile to in y-axis.
     * @param dispResX The display resolution width. Needed for potential re-insertion.
     * @param dispResY The display resolution height. Needed for potential re-insertion.
     */
    void update(int16_t const& tilePosX, int16_t const& tilePosY, uint16_t const& dispResX, uint16_t const& dispResY);

    /**
     * @brief Gets the vector of batches at the specified tile position.
     * @param position The tile position to query: (x, y).
     * @return A reference to the vector of batches at the specified position.
     */
    std::vector<Batch>& getContainerAt(std::pair<uint16_t, uint16_t> const& position) {
        return ObjectContainer[position];
    }

    /**
     * @brief Retrieves a RenderObject from the container by its unique ID.
     *        Does not remove the object from the container.
     *        Do **not** delete the returned object,
     *        it's still owned and managed by the container!
     * @param id The unique ID of the RenderObject to retrieve.
     * @return Pointer to the RenderObject if found, nullptr otherwise.
     */
    RenderObject* getObjectFromId(uint32_t const& id) {
        // Go through all batches
        for (auto& batches : std::views::values(ObjectContainer)) {
            for (auto& [objects, _] : batches) {
                for (auto const& object : objects) {
                    if (object->getId() == id) {
                        return object;
                    }
                }
            }
        }
        return nullptr; // Not found
    }

private:
    /**
     * @brief Holds all objects in the container.
     *        `ObjectContainer[tileX,tileY] -> vector<batch>`
     */
    absl::flat_hash_map<std::pair<int16_t, int16_t>, std::vector<Batch>> ObjectContainer;

    /**
     * @brief Holds all batch worker threads.
     */
    std::vector<std::thread> batchWorkers;

    /**
     * @brief Creates a worker thread for processing a batch.
     * @param work Reference to the batch to process.
     * @param pos The tile position of the batch: (x, y).
     * @param dispResX Display resolution width for tile placement.
     * @param dispResY Display resolution height for tile placement.
     * @return The created worker thread.
     */
    std::thread createBatchWorker(Batch& work, std::pair<int16_t, int16_t> pos, uint16_t dispResX, uint16_t dispResY);

    /**
     * @struct Nebulite::Core::RenderObjectContainer::ReinsertionProcess
     * @brief Holds all objects that are awaiting re-insertion into the container.
     *        The reinsertion process is a 3-step pipeline that ensures objects are properly
     *        re-evaluated and placed back into the correct tile and batch:
     *        - Remove from current batch
     *        - Collect in queue
     *        - Reinsert into the correct tile and batch
     */
    struct ReinsertionProcess {
        std::vector<RenderObject*> queue;
        std::mutex reinsertMutex;
    } reinsertionProcess;

    /**
     * @struct DeletionProcess
     * @brief Manages the deletion process of RenderObjects.
     *        This struct is responsible for handling the various stages of object deletion,
     *        including marking objects for deletion, moving them to trash, and finally
     *        purging them from memory.
     *        The process is a 4-step pipeline that ensures safe and efficient deletion:
     *        - Mark for deletion
     *        - Move to trash
     *        - Move to purgatory
     *        - Delete
     *        Just trash should be enough to resolve all existing references, but we keep this structure for now.
     *        Perhaps in the future we wish to add a restore option, meaning we don't delete purgatory right away.
     *        Or new mechanisms that require a 2-step deletion.
     */
    struct DeletionProcess {
        //std::vector<Nebulite::Core::RenderObject*> to_delete;
        std::vector<RenderObject*> trash; // Moving objects, marking for deletion
        std::vector<RenderObject*> purgatory; // Deleted each frame
        std::mutex deleteMutex; // Threadsafe insertion into trash
    } deletionProcess;
};
} // namespace Nebulite::Core
#endif // NEBULITE_CORE_RENDER_OBJECT_CONTAINER_HPP
