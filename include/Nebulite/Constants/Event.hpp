#ifndef CONSTANTS_EVENT_HPP
#define CONSTANTS_EVENT_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT

//------------------------------------------
namespace Nebulite::Constants {
/**
 * @brief Simple status return value for Domain-Related Nebulite functions such as bound functions or update routines
 */
enum class Event : std::uint8_t {
    success = 0,
    warning,
    error,
};
} // namespace Nebulite::Constants
#endif // CONSTANTS_EVENT_HPP
