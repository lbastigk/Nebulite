#ifndef NEBULITE_DATA_DOCUMENT_SIMPLEVALUEERROR_HPP
#define NEBULITE_DATA_DOCUMENT_SIMPLEVALUEERROR_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT

//------------------------------------------
// Enum for simple value retrieval error

namespace Nebulite::Data {
/**
 * @enum SimpleValueRetrievalError
 * @brief An enumeration representing possible errors that can occur during the retrieval of a simple value from a JSON document.
 * @details This enum is used to indicate specific reasons for failure when attempting to retrieve a simple value,
 *          such as type mismatches, malformed keys, or transformation failures.
 */
enum class SimpleValueRetrievalError : std::uint8_t {
    transformationFailure, // The specified transformations could not be applied successfully.
    conversionFailure,     // The value could not be converted to the requested type.
    malformedKey,          // The provided key is malformed
    isArray,               // The specified key corresponds to an array, not a simple value.
    isObject,              // The specified key corresponds to an object, not a simple value.
    isNull,                // The specified key is not a member of the JSON document (null).
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_SIMPLEVALUEERROR_HPP
