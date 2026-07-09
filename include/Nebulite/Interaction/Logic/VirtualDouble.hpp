#ifndef NEBULITE_INTERACTION_LOGIC_VIRTUALDOUBLE_HPP
#define NEBULITE_INTERACTION_LOGIC_VIRTUALDOUBLE_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Data/Document/ScopedKey.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Interaction::Logic {
/**
 * @brief A wrapper class for accessing and modifying double values in JSON documents.
 * @details The `VirtualDouble` class provides a consistent interface for linking variables to
 *          physical double addresses, ensuring compatibility with `tinyexpr`. It supports:
 *          - Remanent contexts: For `self`, `global`, where the context remains constant.
 *          - Non-remanent contexts: For `other` and `documentCache`,
 *          where the context changes dynamically during evaluations
 *          or the lifetime is limited.
 *          This distinction is crucial for efficient and accurate expression evaluations.
 * @todo Perhaps a better name would be LinkedDouble or LinkedNumericValue
 */
class VirtualDouble {
    // We must store both the key and the scopedkey. Using a string + ScopedKeyView is more memory efficient.
    // Using just ScopedKey is brittle, as returning a string const& of the inner key isn't possible there.

    // Key associated with this VirtualDouble
    std::string const key;

    // Key as ScopedKey with no prefix
    Data::ScopedKeyView const scopedKey;

    /**
     * @brief Internal cache for non-remanent documents.
     */
    double copiedValue = 0.0;

    /**
     * @brief Reference to a value outside
     * @details Represents the first context that was provided
     */
    double* externalReference = nullptr;

public:
    /**
     * @brief Construct a new VirtualDouble object.
     * @details This constructor initializes the VirtualDouble with a key and a DocumentCache.
     *          It also removes any prefixes from the key to ensure consistent access.
     * @param k The key associated with this VirtualDouble.
     */
    explicit VirtualDouble(std::string_view k);

    /**
     * @brief Get the key associated with this VirtualDouble.
     * @return The key as a string.
     */
    [[nodiscard]] std::string const& getKey() const noexcept {
        return key;
    }

    [[nodiscard]] Data::ScopedKeyView const& getScopedKey() const noexcept {
        return scopedKey;
    }

    /**
     * @brief Links the VirtualDouble to an external cache in a JSON document.
     * @details Can only be called once
     * @param json The JSON document to link to.
     */
    void linkExternalCache(Data::JsonScope const& json);

    /**
     * @brief Copies the value from the linked external cache.
     */
    void copyExternalCache();

    /**
     * @brief Copies the value from another JSON document.
     * @param json The JSON document to copy from.
     */
    void copyFromJson(Data::JsonScope const& json);

    /**
     * @brief Set the value of the VirtualDouble directly.
     * @param val The new double value to set.
     */
    void setDirect(double val) noexcept ;

    /**
     * @brief Get a pointer to the linked double
     * @return A pointer to the double value.
     */
    [[nodiscard]] double* ptr() noexcept {
        return &copiedValue;
    }
};
}   // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_VIRTUALDOUBLE_HPP
