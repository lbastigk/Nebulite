#ifndef NEBULITE_DATA_DOCUMENT_KEYTYPE_HPP
#define NEBULITE_DATA_DOCUMENT_KEYTYPE_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT

//------------------------------------------
namespace Nebulite::Data {
/**
 * @enum KeyType
 * @brief Enum representing the type stored of a key in the JSON document.
 */
enum class KeyType : std::uint8_t {
    null = 0,
    value,
    array,
    object
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_KEYTYPE_HPP
