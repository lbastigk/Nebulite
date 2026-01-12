#ifndef NEBULITE_DATA_DOCUMENT_KEY_TYPE_HPP
#define NEBULITE_DATA_DOCUMENT_KEY_TYPE_HPP

#include <cstdint>

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
