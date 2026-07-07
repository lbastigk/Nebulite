#ifndef NEBULITE_DATA_TILING_HPP
#define NEBULITE_DATA_TILING_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <ranges>
#include <vector>

// External
#include <SDL3/SDL_render.h>

// Nebulite
#include "Nebulite/Data/Batch.hpp"
#include "Nebulite/Math/Vec2.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Data {

using TilingInformation = Math::Vec2<std::uint16_t, Math::CoordinateType::WH>;
using TileCoordinate = Math::Vec2<std::int16_t>;

class Tile {
    //------------------------------------------
    // Members

    std::vector<Batch> batches; // While we no longer pass batches to the RendererProcessor, this structuring makes removing objects easier (less searching)
    SDL_Texture* texture = nullptr;

    void deleteTexture();
public:
    //------------------------------------------
    // Constants

    /**
     * @brief Target cost of each Render::update thread batch.
     * @details Set to 0 to disable dynamic batching and process all members per tile in a single thread
     */
    static auto constexpr batchCostGoal = 256;

    //------------------------------------------
    // Methods

    /**
     * @brief Allows read-only access to the internal batches of RenderObjects
     * @return A const reference to the vector of batches contained in this tile. Each batch contains a vector of RenderObjects and an estimated cost.
     */
    [[nodiscard]] std::vector<Batch> const& getBatches() const ;

    /**
     * @brief Allows read-only access to the internal batches of RenderObjects, returning only the vectors of RenderObjects without their associated costs.
     * @return A range of const references to the vectors of RenderObjects contained in each batch of this tile, without their associated costs.
     */
    [[nodiscard]] auto getBatchedObjects() const {
        return getBatches() | std::views::transform([](Batch const& b) -> auto const& { return b.objects; });
    }

    /**
     * @brief Adds a provided batch to the existing batches
     * @param batch The batch to add
     */
    void appendBatch(Batch&& batch);

    /**
     * @brief Moves all objects in the tile to the provided destination vector and clears the tile's batches.
     * @param destination The vector to move the objects to. Objects will be moved (not copied) and the tile's batches will be cleared.
     */
    void moveObjects(std::vector<Core::RenderObject*>& destination);

    /**
     * @brief Inserts an object into the first batch that has an estimated cost less than or equal to the batchCostGoal.
     * @param toAppend The object to add
     * @return True if a batch was found (object inserted), false if no batch was found (object not inserted)
     */
    bool insertIfCostGoalMatches(Core::RenderObject* toAppend);

    /**
     * @brief Updates all objects of this tile
     * @param to_move Objects moved out of the tile during the update
     * @param to_delete Objects that were deleted during the update
     * @param tilingInfo The pixel height/width of each tile
     * @param coordinate The coordinate of this tile
     */
    void update(std::vector<Core::RenderObject*>& to_move, std::vector<Core::RenderObject*>& to_delete, TilingInformation const& tilingInfo, TileCoordinate const& coordinate);

    /**
     * @brief Renders the tile to the screen utilizing a pre-rendered texture
     * @details If this tile has no pre-rendered texture, the texture is generated first, then rendered to the screen.
     *          If a texture already exists, it is directly rendered to the screen.
     *          The texture is invalidated and re-rendered whenever an object is added, removed, or moved from the tile.
     *          Note that this should only be used for tiles that have no animated objects (e.g., background layer),
     *          as animated objects would require constant re-rendering of the texture, which would be inefficient.
     * @param nebuliteRenderer Nebulites renderer
     * @param coordinate The coordinate of this tile
     * @param tilingInfo The pixel height/width of each tile
     * @param capture The capture instance for logging errors during texture creation and rendering
     * @param dispPosX The display position, horizontally
     * @param dispPosY The display position, vertically
     * @param windowScale The scaling factor of the window
     */
    void render(
        Core::Renderer const& nebuliteRenderer,
        TileCoordinate const& coordinate,
        TilingInformation const& tilingInfo,
        Utility::IO::Capture& capture,
        int dispPosX,
        int dispPosY,
        int windowScale
    );
};

} // namespace Nebulite::Data
#endif // NEBULITE_DATA_TILING_HPP
