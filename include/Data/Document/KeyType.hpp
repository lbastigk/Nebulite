#ifndef NEBULITE_DATA_DOCUMENT_KEY_TYPE_HPP
#define NEBULITE_DATA_DOCUMENT_KEY_TYPE_HPP

// NOLINTNTEXTLINE
#include <cstdint> // For some reason clang-tidy flags the include as unnecessary

namespace Nebulite::Data {
/**
 * @enum KeyType
 * @brief Enum representing the type stored of a key in the JSON document.
 */
enum class KeyType : uint8_t {
    null,
    value,
    array,
    object
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_KEY_TYPE_HPP
