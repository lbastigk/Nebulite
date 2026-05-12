#ifndef DATA_DOCUMENT_KEYTYPE_HPP
#define DATA_DOCUMENT_KEYTYPE_HPP

namespace Nebulite::Data {
/**
 * @enum KeyType
 * @brief Enum representing the type stored of a key in the JSON document.
 */
enum class KeyType : std::uint8_t {
    null,
    value,
    array,
    object
};
} // namespace Nebulite::Data
#endif // DATA_DOCUMENT_KEYTYPE_HPP
