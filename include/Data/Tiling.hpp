#ifndef DATA_TILING_HPP
#define DATA_TILING_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINTTHISLINE
#include <utility>

// External
#include <SDL3/SDL_render.h>

// Nebulite
#include "Data/Batch.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
namespace Nebulite::Data {

struct TilingInformation {
    std::uint16_t w;
    std::uint16_t h;
};

struct TileCoordinate {
    std::int16_t x;
    std::int16_t y;

    TileCoordinate() : x(0), y(0) {}

    TileCoordinate(std::int16_t const xVal, std::int16_t const yVal) : x(xVal), y(yVal) {}

    // Add hashing capability
    bool operator==(const TileCoordinate& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const TileCoordinate& other) const {
        return x != other.x || y != other.y;
    }

    template <typename H>
    friend H AbslHashValue(H h, const TileCoordinate& coord) {
        return H::combine(std::move(h), coord.x, coord.y);
    }
};

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

    std::vector<Batch> const& getBatches() const ;

    void clearBatches();

    void appendBatch(Batch const& batch);

    void moveObjects(std::vector<Core::RenderObject*>& destination);

    bool insertIfCostGoalMatches(Core::RenderObject* toAppend);

    void update(std::vector<Core::RenderObject*>& to_move, std::vector<Core::RenderObject*>& to_delete, TilingInformation const& tilingInformation, TileCoordinate const& coord);

    SDL_Texture*& getTexture();
};

} // namespace Nebulite::Data
#endif // DATA_TILING_HPP
