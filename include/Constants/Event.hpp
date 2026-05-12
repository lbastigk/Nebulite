#pragma once

//------------------------------------------
// Includes

// Standard library
// NOLINTNEXTLINE
#include <cstdint> // somehow flagged as not used

//------------------------------------------
namespace Nebulite::Constants {

enum class Event : std::uint8_t {
    Success = 0,
    Warning,
    Error
};

} // namespace Nebulite::Constants
