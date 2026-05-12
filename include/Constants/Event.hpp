#ifndef CONSTANTS_EVENT_HPP
#define CONSTANTS_EVENT_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINTTHISLINE

//------------------------------------------
namespace Nebulite::Constants {

enum class Event : uint8_t {
    Success = 0,
    Warning,
    Error
};

} // namespace Nebulite::Constants

#endif // CONSTANTS_EVENT_HPP
