#ifndef NEBULITE_DATA_RENDERER_PROCESSOR_HPP
#define NEBULITE_DATA_RENDERER_PROCESSOR_HPP

#include <array>
#include <atomic>
#include <memory>
#include <vector>

#include "Constants/ThreadSettings.hpp"
#include "Utility/Coordination/WorkDispatcher.hpp"

namespace Nebulite::Core {
class RenderObject;
} // namespace Nebulite::Core

namespace Nebulite::Data {
class RenderObjectContainer;
} // namespace Nebulite::Data

namespace Nebulite::Data {
/**
 * @struct Nebulite::Data::ReinsertionProcess
 * @brief Holds all objects that are awaiting re-insertion into the container.
 *        The reinsertion process is a 3-step pipeline that ensures objects are properly
 *        re-evaluated and placed back into the correct tile and batch:
 *        - Remove from current batch
 *        - Collect in queue
 *        - Reinsert into the correct tile and batch
 */
struct ReinsertionProcess {
    std::vector<Core::RenderObject*> queue;
    std::mutex reinsertMutex;
};

/**
 * @struct Nebulite::Data::DeletionProcess
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
    std::vector<Core::RenderObject*> trash; // Moving objects, marking for deletion
    std::vector<Core::RenderObject*> purgatory; // Deleted each frame
    std::mutex deleteMutex; // Threadsafe insertion into trash
};

/**
 * @struct Nebulite::Data::Batch
 * @brief Represents a batch of RenderObject instances in a given tile.
 *        `Batch -> vector<RenderObject*>`
 *        Used for threading and parallel processing of render objects.
 *        Basically a custom std::vector wrapper for easier cost management.
 */
struct Batch {
    // Collection of RenderObjects
    std::vector<Core::RenderObject*> objects;

    // Full estimated cost of the batch
    uint64_t estimatedCost = 0;

    /**
     * @brief Pops the last RenderObject from the batch.
     * @return Pointer to the popped RenderObject, or nullptr if batch is already empty.
     */
    Core::RenderObject* pop();

    /**
     * @brief Pushes a RenderObject into the batch.
     * @param obj Pointer to the RenderObject to push.
     */
    void push(Core::RenderObject* obj);

    /**
     * @brief Removes a RenderObject from the batch.
     * @param obj Pointer to the RenderObject to remove.
     * @return True if the object was removed, false otherwise.
     */
    bool removeObject(Core::RenderObject* obj);
};

/**
 * @class Nebulite::Data::RendererProcessor
 * @brief Manages the processing of RenderObjects for rendering.
 *        This class is responsible for preparing RenderObjects for rendering by organizing them into batches,
 *        managing worker threads for batch processing, and handling reinsertion and deletion processes.
 */
class RendererProcessor {
public:
    RendererProcessor();

    ~RendererProcessor();

    // Non-copyable, non-movable
    RendererProcessor(RendererProcessor const&) = delete;
    RendererProcessor& operator=(RendererProcessor const&) = delete;
    RendererProcessor(RendererProcessor&&) = delete;
    RendererProcessor& operator=(RendererProcessor&&) = delete;

    /**
     * @brief Prepares the RendererProcessor for processing a new layer of RenderObjects
     *        by setting all worker threads referencing the new layer.
     * @param layer The RenderObjectContainer representing the new layer to process.
     */
    void prepareForNewLayer(RenderObjectContainer* layer) const ;

    /**
     * @brief Flag to signal threads to stop.
     */
    std::atomic<bool> stopFlag;

    /**
     * @brief Workspace struct for batch worker threads.
     */
    struct DispatcherWorkspace {
        std::vector<Batch*> work;
        int16_t tilePosX;
        int16_t tilePosY;
        uint16_t dispResX;
        uint16_t dispResY;
        std::pair<uint16_t, uint16_t> pos;
        ReinsertionProcess* reinsertionProcess;
        DeletionProcess* deletionProcess;
        uint32_t cost = 0;
    };

    /**
     * @brief Worker function for processing batches in parallel.
     * @param workspace The workspace containing the batches to process and necessary context information.
     */
    static void batchWorkerFunc(DispatcherWorkspace const& workspace);

    /**
     * @brief Holds all batch worker threads.
     * @details Pool of pre-initialized workers for reuse
     */
    std::array<
        std::unique_ptr<Utility::Coordination::WorkDispatcher<DispatcherWorkspace>>,
        Constants::ThreadSettings::Maximum::rendererWorkerCount
    > batchWorkerPool;

    /**
     * @brief Starts all batch worker threads to process the assigned batches in parallel.
     */
    void processPool() const ;
};

} // namespace Nebulite::Data
#endif // NEBULITE_DATA_RENDERER_PROCESSOR_HPP
