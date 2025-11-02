/**
 * @file VirtualDouble.hpp
 * @brief Provides a wrapper for consistent access to double values in JSON documents.
 * 
 * This file contains the `VirtualDouble` class, which ensures compatibility with `tinyexpr`
 * by linking variables to physical double addresses. It supports both remanent and non-remanent
 * contexts for efficient expression evaluation.
 */

#ifndef NEBULITE_INTERACTION_LOGIC_VIRTUALDOUBLE_HPP
#define NEBULITE_INTERACTION_LOGIC_VIRTUALDOUBLE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Utility/DocumentCache.hpp"
#include "Utility/JSON.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {
/**
 * @brief A wrapper class for accessing and modifying double values in JSON documents.
 * 
 * The `VirtualDouble` class provides a consistent interface for linking variables to
 * physical double addresses, ensuring compatibility with `tinyexpr`. It supports:
 * 
 * - Remanent contexts: For `self`, `global`, where the context remains constant.
 * 
 * - Non-remanent contexts: For `other` and `documentcache`, 
 * where the context changes dynamically during evaluations
 * or the lifetime is limited.
 * 
 * This distinction is crucial for efficient and accurate expression evaluations.
 */
class VirtualDouble {
private:
    // Linked Read-Only cache
    Nebulite::Utility::DocumentCache* documentCache = nullptr;

    // Key associated with this VirtualDouble
    std::string key;

    /**
     * @brief Internal cache for non-remanent documents.
     */
    double copied_value = 0.0;

    /**
     * @brief Pointer to the actual double value.
     * 
     * Initially points to the internal cache for non-remanent documents.
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
     * 
     * This constructor initializes the VirtualDouble with a key and a DocumentCache.
     * It also removes any prefixes from the key to ensure consistent access.
     * 
     * @param k The key associated with this VirtualDouble.
     * @param documentCache The DocumentCache to use for retrieving values.
     */
    VirtualDouble(std::string k, Nebulite::Utility::DocumentCache* documentCache) noexcept
    : documentCache(documentCache), key(std::move(k))
    {
        if      (key.starts_with(ContextPrefix::self))   { key.erase(0, ContextPrefix::self.size());   }
        else if (key.starts_with(ContextPrefix::other))  { key.erase(0, ContextPrefix::other.size());  }
        else if (key.starts_with(ContextPrefix::global)) { key.erase(0, ContextPrefix::global.size()); }
    }

    /**
     * @brief Get the key associated with this VirtualDouble.
     * 
     * @return The key as a string.
     */
    std::string const& getKey() const noexcept {
        return key;
    }

    /**
     * @brief Update the cache value from the JSON document or DocumentCache.
     * 
     * This function retrieves the double value associated with the key from the provided JSON document
     * or the DocumentCache, and updates the internal cache accordingly.
     * This is used for non-remanent documents, meaning the associated document changes
     * 
     * If the key is not found within the associated document, the double value will default to 0.
     * 
     * @param json The JSON document pointer to retrieve the value from. If the pointer is null, we retrieve the value from the document cache.
     */
    void setUpInternalCache(Nebulite::Utility::JSON* json){
        if (json != nullptr){
            copied_value = *json->getStableDoublePointer(key);
            reference = &copied_value;
        }
        else if (documentCache != nullptr){
            copied_value = *documentCache->getStableDoublePointer(key);
            reference = &copied_value;
        }
    }

    /**
     * @brief Register the external cache for this VirtualDouble.
     * 
     * This function links the VirtualDouble to an external double pointer of a JSON document, instead of using its internal cache.
     * allowing it to access and modify the value directly.
     */
    void setUpExternalCache(Nebulite::Utility::JSON* json){
        if (json != nullptr){
            reference = json->getStableDoublePointer(key);
        }
        else if (documentCache != nullptr){
            reference = documentCache->getStableDoublePointer(key);
        }
    }

    /**
     * @brief Set the value of the VirtualDouble directly.
     * 
     * This function updates the internal cache value directly.
     * Used for non-remanent documents where we want to set a value without linking to an external source.
     * 
     * Assumes that the reference already points to the internal cache!
     * 
     * @param val The new double value to set.
     */
    void setDirect(double val) noexcept {
        copied_value = val;
    }

    /**
     * @brief Get a pointer to the linked double
     * 
     * Depending on type of linkage, this is either:
     * 
     * - internally for documents changing context
     * 
     * - externally for remanent documents
     * 
     * @return A pointer to the double value.
     */
    double* ptr() const noexcept {
        return reference;
    }
};
}   // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_VIRTUALDOUBLE_HPP