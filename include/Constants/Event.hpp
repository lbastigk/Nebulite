#pragma once

//------------------------------------------
// Includes

// Standard library
#include <cstdint>

//------------------------------------------
namespace Nebulite::Constants {

enum class Event : std::uint8_t {
    Success = 0,
    Warning,
    Error
};

} // namespace Nebulite::Constants
