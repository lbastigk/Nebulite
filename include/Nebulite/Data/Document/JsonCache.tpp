#ifndef NEBULITE_DATA_DOCUMENT_JSONCACHE_TPP
#define NEBULITE_DATA_DOCUMENT_JSONCACHE_TPP

//------------------------------------------
// Includes

// Standard library
#include <optional>

// Nebulite
#include "Nebulite/Data/Document/RjDirectAccess.hpp"

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_DATA_DOCUMENT_JSONCACHE_HPP
    #include "Nebulite/Data/Document/JsonCache.hpp"
#endif // NEBULITE_DATA_DOCUMENT_JSONCACHE_HPP

//------------------------------------------
namespace Nebulite::Data {
/**
 * @brief Helper function to convert any type from cache into another type.
 * @return The converted value of type NewType, or nullopt if conversion fails.
 */
template <typename NewType>
std::optional<NewType> CacheEntry::convertTo(){
    return RjDirectAccess::convertSimpleValue<NewType>(value);
}
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSONCACHE_TPP
