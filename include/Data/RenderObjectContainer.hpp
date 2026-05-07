/**
 * @file RenderObjectContainer.hpp
 * @brief Contains the Nebulite::Data::RenderObjectContainer class.
 */

#ifndef NEBULITE_DATA_RENDER_OBJECT_CONTAINER_HPP
#define NEBULITE_DATA_RENDER_OBJECT_CONTAINER_HPP

//------------------------------------------
// Includes

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Data/RendererProcessor.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class RenderObject;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Data {
/**
 * @class Nebulite::Data::RenderObjectContainer
 * @brief Manages a collection of RenderObject instances in a tile-based container.
 */
class RenderObjectContainer {
public:
    //------------------------------------------
    // Special member functions

    RenderObjectContainer() = default;

    ~RenderObjectContainer() = default;

    // Non-copyable, non-movable
    RenderObjectContainer(RenderObjectContainer const&) = delete;
    RenderObjectContainer(RenderObjectContainer&&) = delete;
    RenderObjectContainer& operator=(RenderObjectContainer const&) = delete;
    RenderObjectContainer& operator=(RenderObjectContainer&&) = delete;

    //------------------------------------------
    // Constants

    /**
     * @brief Target cost of each Render::update thread batch.
     * @details Set to 0 to disable dynamic batching and process all members per tile in a single thread
     */
    static auto constexpr batchCostGoal = 256;

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
     * @param capture Capture instance to pass to RenderObjects during construction.
     */
    void deserialize(std::string const& serialOrLink, uint16_t const& dispResX, uint16_t const& dispResY, Utility::IO::Capture& capture);

    //------------------------------------------
    // Pipeline

    /**
     * @brief Appends a RenderObject to the container.
     *        Places it in the appropriate tile and batches it through cost-estimation.
     * @param toAppend Pointer to the RenderObject to append.
     * @param dispResX Display resolution width for tile placement.
     * @param dispResY Display resolution height for tile placement.
     */
    void append(Core::RenderObject* toAppend, uint16_t const& dispResX, uint16_t const& dispResY);

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
    bool isValidPosition(TileCoordinate const& position) const ;

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
     * @param tiles The vector of tile coordinates that are currently within the viewport and need to be updated.
     * @param dispResX The display resolution width. Needed for potential re-insertion.
     * @param dispResY The display resolution height. Needed for potential re-insertion.
     * @param rendererProcessor The RendererProcessor instance to use for parallel processing of batches.
     */
    void update(std::vector<TileCoordinate> const& tiles, uint16_t const& dispResX, uint16_t const& dispResY, RendererProcessor const& rendererProcessor);

    /**
     * @brief Gets the vector of batches at the specified tile position.
     * @param position The tile position to query: (x, y).
     * @return A reference to the vector of batches at the specified position.
     */
    std::vector<Batch>& getContainerAt(TileCoordinate const& position) {
        return ObjectContainer[position];
    }

    /**
     * @brief Retrieves a RenderObject from the container by its unique ID.
     *        Does not remove the object from the container.
     *        Do **not** delete the returned object,
     *        it's still owned and managed by the container!
     * @param domainId The unique ID of the RenderObject to retrieve.
     * @return Pointer to the RenderObject if found, nullptr otherwise.
     */
    Core::RenderObject* getObjectFromId(size_t const& domainId);

    struct ContainerInfo {
        // Container stats
        size_t containerTotalTiles = 0;
        size_t containerTotalCost = 0;
    };

    /**
     * @brief Returns information about the internal state of the container for debugging purposes.
     */
    [[nodiscard]] ContainerInfo getContainerInfo() const;

    /**
     * @brief Calculates the corresponding tile position for a given RenderObject based on its coordinates and the display resolution.
     * @param toAppend Pointer to the RenderObject for which to calculate the tile position.
     * @param displayResolutionX The display resolution width, used to determine the tile size and position.
     * @param displayResolutionY The display resolution height, used to determine the tile size and position.
     * @return A pair of int16_t representing the tile position (tileX, tileY) corresponding to the RenderObject's coordinates.
     */
    static TileCoordinate getTilePos(Core::RenderObject const* toAppend, uint16_t const& displayResolutionX, uint16_t const& displayResolutionY);

    /**
     * @brief Process for reinserting objects after they have been moved.
     */
    ReinsertionProcess reinsertionProcess;

    /**
     * @brief Process for deleting objects after they have been marked for deletion.
     */
    DeletionProcess deletionProcess;

    template<typename MetaInfo>
    using IteratorFunction = std::function<void(TileCoordinate const&, MetaInfo const&, std::vector<Batch> const&)>;

    /**
     * @brief iterate over all tile coordinates
     * @tparam MetaInfo type of the meta information to pass to the function
     * @param function iterator function
     * @param metaInfo meta information to pass to the function
     */
    template<typename MetaInfo>
    void containerIteration(IteratorFunction<MetaInfo> const& function, MetaInfo const& metaInfo) {
        for (auto const& [tile, batches] : ObjectContainer) {
            function(tile, metaInfo, batches);
        }
    }

private:
    /**
     * @brief Holds all objects in the container.
     *        `ObjectContainer[tileX,tileY] -> vector<batch>`
     */
    absl::flat_hash_map<TileCoordinate, std::vector<Batch>> ObjectContainer;
};
} // namespace Nebulite::Core
#endif // NEBULITE_DATA_RENDER_OBJECT_CONTAINER_HPP
