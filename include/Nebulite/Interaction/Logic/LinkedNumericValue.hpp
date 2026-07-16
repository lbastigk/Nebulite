#ifndef NEBULITE_INTERACTION_LOGIC_VIRTUALDOUBLE_HPP
#define NEBULITE_INTERACTION_LOGIC_VIRTUALDOUBLE_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Data/Document/ScopedKeyView.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Interaction::Logic {
/**
 * @brief A wrapper class for accessing and modifying double values in JSON documents.
 * @details The `LinkedNumericValue` class provides a consistent interface for linking variables to
 *          physical double addresses, ensuring compatibility with `tinyexpr`. It supports:
 *          - Remanent contexts: For `self`, `global`, where the context remains constant.
 *          - Non-remanent contexts: For `other` and `documentCache`,
 *          where the context changes dynamically during evaluations
 *          or the lifetime is limited.
 *          This distinction is crucial for efficient and accurate expression evaluations.
 */
class LinkedNumericValue {
    // We must store both the key and the scopedkey. Using a string + ScopedKeyView is more memory efficient.
    // Using just ScopedKey is brittle, as returning a string const& of the inner key isn't possible there.

    // Key associated with this LinkedNumericValue
    std::string const key;

    // Key as ScopedKey with no prefix
    Data::ScopedKeyView const scopedKey;

    /**
     * @brief Internal cache for non-remanent documents.
     * @details Pointer instead of reference so the class remains standard-layout
     */
    double* reference;

public:
    /**
     * @brief Construct a new LinkedNumericValue object.
     * @details This constructor initializes the LinkedNumericValue with a key and a DocumentCache.
     *          It also removes any prefixes from the key to ensure consistent access.
     * @param k The key associated with this LinkedNumericValue.
     * @param r A pointer to the double value to link.
     */
    explicit LinkedNumericValue(std::string_view k, double& r);

    /**
     * @brief Get the key associated with this LinkedNumericValue.
     * @return The key as a string.
     */
    [[nodiscard]] std::string const& getKey() const noexcept {
        return key;
    }

    [[nodiscard]] Data::ScopedKeyView const& getScopedKey() const noexcept {
        return scopedKey;
    }

    /**
     * @brief Copies the value from another JSON document.
     * @param json The JSON document to copy from.
     */
    void copyFromJson(Data::JsonScope const& json);

    /**
     * @brief Set the value of the LinkedNumericValue directly.
     * @param val The new double value to set.
     */
    void setDirect(double val) noexcept ;
};
}   // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_VIRTUALDOUBLE_HPP
