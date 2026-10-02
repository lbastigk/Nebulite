#ifndef NEBULITE_UTILITY_TESTING_HPP
#define NEBULITE_UTILITY_TESTING_HPP

//------------------------------------------
// Includes

// Standard library
#include <stdexcept>

// Nebulite
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @brief Checks if a condition is true, throws a runtime error with a custom message if not.
 * @tparam MessageArgs The Argument types forming the message
 * @param condition The condition to check
 * @param args The Arguments forming the message
 */
template <typename... MessageArgs>
void assume(bool const condition, MessageArgs&&... args) {
    if (!condition) {
        auto message = StringHandler::combineWithNewline(std::forward<MessageArgs>(args)...);
        throw std::runtime_error(message);
    }
}
} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_TESTING_HPP
