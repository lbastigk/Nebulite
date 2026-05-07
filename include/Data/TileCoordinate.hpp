#pragma once

//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <utility>

//------------------------------------------
namespace Nebulite::Data {

struct TileCoordinate {
    int16_t x;
    int16_t y;

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

} // namespace Nebulite::Data
