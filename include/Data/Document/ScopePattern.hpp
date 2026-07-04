#ifndef DATA_DOCUMENT_SCOPEPATTERN_HPP
#define DATA_DOCUMENT_SCOPEPATTERN_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/OptionalFixedString.hpp"

//------------------------------------------
namespace Nebulite::Data {
/**
 * @brief Optional Fixed Strings to be used for the Data::KeyGroup class to deduct the scope of its keys.
 */
class ScopePattern {
public:
    /**
     * @brief A constant representing the absence of a scope.
     */
    static auto constexpr noScope = OptionalFixedString();

    /**
     * @brief The entire scope is unknown at compile time, but to be used at a given root.
     */
    static auto constexpr domainRootScope = OptionalFixedString<0, true>();
};
} // namespace Nebulite::Data
#endif // DATA_DOCUMENT_SCOPEPATTERN_HPP
