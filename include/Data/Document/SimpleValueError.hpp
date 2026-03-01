#ifndef SIMPLE_VALUE_ERROR_HPP
#define SIMPLE_VALUE_ERROR_HPP

//------------------------------------------
// Enum for simple value retrieval error

namespace Nebulite::Data {

/**
 * @enum SimpleValueRetrievalError
 * @brief An enumeration representing possible errors that can occur during the retrieval of a simple value from a JSON document.
 * @details This enum is used to indicate specific reasons for failure when attempting to retrieve a simple value,
 *          such as type mismatches, malformed keys, or transformation failures.
 */
enum SimpleValueRetrievalError {
    TRANSFORMATION_FAILURE, // The specified transformations could not be applied successfully.
    CONVERSION_FAILURE,      // The value could not be converted to the requested type.
    MALFORMED_KEY,
    IS_ARRAY, // The specified key corresponds to an array, not a simple value.
    IS_OBJECT, // The specified key corresponds to an object, not a simple value.
    IS_NULL,   // The specified key is not a member of the JSON document (null).
    IS_UNKNOWN_TYPE // The specified key corresponds to a type that is not supported as a simple value
};
} // namespace Nebulite::Data
#endif // SIMPLE_VALUE_ERROR_HPP
