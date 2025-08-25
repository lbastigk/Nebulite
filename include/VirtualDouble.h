/**
 * @file VirtualDouble.h
 * @brief Provides a wrapper for consistent access to double values in JSON documents.
 * 
 * This file defines the `VirtualDouble` class, which ensures compatibility with `tinyexpr`
 * by linking variables to physical double addresses. It supports both remanent and non-remanent
 * contexts for efficient expression evaluation.
 */

#pragma once
#include "JSON.h"
#include "DocumentCache.h"

namespace Nebulite{

/**
 * @brief A wrapper class for accessing and modifying double values in JSON documents.
 * 
 * The `VirtualDouble` class provides a consistent interface for linking variables to
 * physical double addresses, ensuring compatibility with `tinyexpr`. It supports:
 * 
 * - Remanent contexts: For `self`, `global`, and `documentcache`, where the context remains constant.
 * 
 * - Non-remanent contexts: For `other`, where the context changes dynamically during evaluations.
 * 
 * This distinction is crucial for efficient and accurate expression evaluations.
 */
class VirtualDouble {
    // Linked Read-Only cache
    Nebulite::DocumentCache* documentCache = nullptr;

    // Key associated with this VirtualDouble
    std::string key;

    // Internal cache for the double value
    double internal_cache = 0.0;

    // External cache
    double* external_cache = nullptr;
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
    VirtualDouble(const std::string& k, Nebulite::DocumentCache* documentCache) 
        : key(k), documentCache(documentCache) {
            // Removing self/other/global prefixes
            if (key.find("self.") == 0) key = key.substr(5);
            else if (key.find("other.") == 0) key = key.substr(6);
            else if (key.find("global.") == 0) key = key.substr(7);
        }

    /**
     * @brief Get the key associated with this VirtualDouble.
     * 
     * @return The key as a string.
     */
    std::string getKey() {
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
    void updateCache(Nebulite::JSON* json) {
        double fallback_value = 0.0;
        if (json != nullptr) {
            internal_cache = json->get<double>(key.c_str(), fallback_value);
        }
        else if (documentCache != nullptr) {
            internal_cache = documentCache->getData<double>(key.c_str(), fallback_value);
        }
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
    double* ptr(){
        if(external_cache != nullptr) {
            return external_cache;
        }
        return &internal_cache;
    }

    /**
     * @brief Register the external cache for this VirtualDouble.
     * 
     * This function links the VirtualDouble to an external double pointer of a JSON document,
     * allowing it to access and modify the value directly.
     */
    void register_external_double_cache(Nebulite::JSON* json) {
        if (json != nullptr) {
            external_cache = json->getDoublePointerOf(key.c_str());
        }
        else if (documentCache != nullptr) {
            external_cache = documentCache->getDoublePointerOf(key);
        }
    }
};
}   // namespace Nebulite