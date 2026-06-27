#ifndef DATA_BATCH_HPP
#define DATA_BATCH_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <vector>

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class RenderObject;
} // namespace Nebulite::Core

namespace Nebulite::Data {
class RenderObjectContainer;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Data {
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

    void updateCost();

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

} // namespace Nebulite::Data
#endif // DATA_BATCH_HPP
