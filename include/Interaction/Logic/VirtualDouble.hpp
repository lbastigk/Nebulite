/**
 * @file VirtualDouble.hpp
 * @brief Provides a wrapper for consistent access to double values in JSON documents.
 * @details This file contains the `VirtualDouble` class, which ensures compatibility with `tinyexpr`
 *          by linking variables to physical double addresses. It supports both remanent and non-remanent
 *          contexts for efficient expression evaluation.
 */

#ifndef NEBULITE_INTERACTION_LOGIC_VIRTUAL_DOUBLE_HPP
#define NEBULITE_INTERACTION_LOGIC_VIRTUAL_DOUBLE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/DocumentCache.hpp"
#include "Data/Document/JSON.hpp"

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
 * @todo Perhaps a better name would be LinkedDouble
 */
class VirtualDouble {
    // Key associated with this VirtualDouble
    std::string key;

    /**
     * @brief Internal cache for non-remanent documents.
     */
    double copied_value = 0.0;

    /**
     * @brief Pointer to the actual double value.
     * @details Initially points to the internal cache for non-remanent documents.
     */
    double* reference = &copied_value;

    /**
     * @brief Key prefixes for different contexts.
     */
    struct ContextPrefix {
        static constexpr std::string_view self   = "self.";
        static constexpr std::string_view other  = "other.";
        static constexpr std::string_view global = "global.";
    };
    
public:
    /**
     * @brief Construct a new VirtualDouble object.
     * @details This constructor initializes the VirtualDouble with a key and a DocumentCache.
     *          It also removes any prefixes from the key to ensure consistent access.
     * @param k The key associated with this VirtualDouble.
     */
    explicit VirtualDouble(std::string k) noexcept ;

    /**
     * @brief Get the key associated with this VirtualDouble.
     * @return The key as a string.
     */
    [[nodiscard]] std::string const& getKey() const noexcept {
        return key;
    }

    /**
     * @brief Update the cache value from the JSON document or DocumentCache.
     * @details Retrieves the double value associated with the key from the provided JSON document
     *          or the DocumentCache, and updates the internal cache accordingly.
     *          This is used for non-remanent documents, meaning the associated document changes
     *          If the key is not found within the associated document, the double value will default to 0.
     * @param json The JSON document to retrieve the value from.
     */
    void setUpInternalCache(Data::JSON& json);

    /**
     * @brief Update the cache value from the global DocumentCache.
     * @details Retrieves the double value associated with the key from the global DocumentCache,
     *          and updates the internal cache accordingly.
     *          This is used for non-remanent documents, meaning the associated document changes
     *          If the key is not found within the DocumentCache, the double value will default to 0.
     */
    void setUpInternalCache();

    /**
     * @brief Register the external cache for this VirtualDouble.
     * @details Links the VirtualDouble to an external double pointer of a JSON document,
     *          instead of using its internal cache.
     *          Allowing it to access and modify the value directly.
     * @param json The JSON document to retrieve the stable double pointer from.
     */
    void setUpExternalCache(Data::JSON& json);

    /**
     * @brief Set the value of the VirtualDouble directly.
     * @details Updates the internal cache value directly.
     *          Used for non-remanent documents where we want to set a value without linking to an external source.
     *          Assumes that the reference already points to the internal cache!
     * @param val The new double value to set.
     */
    void setDirect(double const& val) noexcept {
        copied_value = val;
    }

    /**
     * @brief Get a pointer to the linked double
     * @details Depending on type of linkage, this is either:
     *          - internally for documents changing context
     *          - externally for remanent documents
     * @return A pointer to the double value.
     */
    [[nodiscard]] double* ptr() const noexcept {
        return reference;
    }
};
}   // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_VIRTUAL_DOUBLE_HPP
