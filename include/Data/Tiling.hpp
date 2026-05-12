#pragma once

//------------------------------------------
// Includes

// Standard library
// NOLINTNEXTLINE
#include <cstdint> // somehow flagged as not used
#include <utility>

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

} // namespace Nebulite::Data
