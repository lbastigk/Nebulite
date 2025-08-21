/**
 * @file DocumentCache.h
 * 
 * @brief Caches documents as read-only for quick access of resources.
 * 
 * This file contains the DocumentCache class for managing cached, read-only documents.
 */

#pragma once

#include "JSON.h"
#include "FileManagement.h"

/*
TODO: Unloading of documents

The basic idea is to add a timekeeper for all loaded documents, tracking their last usage.
This, however, results in a O(n) complexity, slowing down the retrieval of documents as the only place to update the last used time is during retrieval.

The idea instead is to use a random approach, checking one document at random during each retrieval.
If the document is found to be unused for a certain period, it can be unloaded from memory.

This is possible, as the exact time of unloading is unimportant.

Furthermore, we might wish to set a minimum threshold for how many documents must be loaded before we start unloading them.
This would prevent us from unloading documents too aggressively and potentially impacting performance.
*/

namespace Nebulite {

/**
 * @class Nebulite::DocumentCache
 * @brief Caches documents as read-only for quick access of resources.
 * 
 * This class provides methods to retrieve data from cached documents efficiently.
 * It allows for quick access to frequently used data without the need to reload documents.
 * It also manages the loading of documents as needed.
 * 
 * @todo Unloading of documents is currently not supported.
 */
class DocumentCache{
public:
    DocumentCache() = default;

    /**
     * @brief Retrieves data from a cached document.
     * 
     * Used to quickly access frequently used data without reloading the document. Unloaded documents are automatically loaded and cached.
     * 
     * @tparam T The type of the data to retrieve.
     * 
     * @param doc_key The key identifying the document and the specific data to retrieve: <linkToDocument>:<key>
     * 
     * @param defaultValue The value to return if the document or key is not found.
     * 
     * @return The retrieved data or the default value as type T.
     */
    template  <typename T> 
    T getData(std::string doc_key, const T& defaultValue = T());

    /**
     * @brief Retrieves a pointer to a double value from a cached document.
     * 
     * This function provides a way to access a remanent double pointer from a cached document.
     * This is used to compile tinyexpr expressions with direct linkage to in-document values.
     * 
     * @param doc_key The key identifying the document and the specific data to retrieve: <linkToDocument>:<key>
     * 
     * @return A pointer to the retrieved double value. 
     * Guaranteed to be valid even if the key does not exist within the document,
     * or if the document itself is not found!
     */
    double* getDoublePointerOf(const std::string& doc_key) {
        // Split the input into document name and key
        size_t pos = doc_key.find(':');
        if (pos == std::string::npos) {
            return &zero; // Return a pointer to zero if format is incorrect
        }
        std::string doc = doc_key.substr(0, pos);
        std::string key = doc_key.substr(pos + 1);

        if(ReadOnlyDocs.find(doc) == ReadOnlyDocs.end()) {
            // Load the document if it doesn't exist
            std::string serial = FileManagement::LoadFile(doc);
            if (serial.empty()) {
                return &zero;
            }
            ReadOnlyDocs[doc].deserialize(serial);
        }

        // Register the external cache
        if (ReadOnlyDocs.find(doc) != ReadOnlyDocs.end()) {
            return ReadOnlyDocs[doc].getDoublePointerOf(key);
        }
        return &zero; // Return a pointer to zero if the document or key is not found
    }
private:
    // Cache for read-only documents
    absl::flat_hash_map<std::string,Nebulite::JSON> ReadOnlyDocs;

    // Default value for double pointers, if the document or key is not found
    double zero = 0.0;
};
}

// Expected input: ./Resources/Data/myData.jsonc:key1.key2
template  <typename T> 
T Nebulite::DocumentCache::getData(std::string doc_key, const T& defaultValue) {
    // Split the input into document name and key
    size_t pos = doc_key.find(':');
    if (pos == std::string::npos) {
        return defaultValue; // Return default value if format is incorrect
    }
    std::string doc = doc_key.substr(0, pos);
    std::string key = doc_key.substr(pos + 1);

    // Check if the document exists in the cache
    if (ReadOnlyDocs.find(doc) == ReadOnlyDocs.end()) {
        // Load the document if it doesn't exist
        std::string serial = FileManagement::LoadFile(doc);
        if (serial.empty()) {
            return defaultValue; // Return default value if document loading fails
        }
        ReadOnlyDocs[doc].deserialize(serial);
    }

    // Return key:
    return ReadOnlyDocs[doc].get<T>(key.c_str(), defaultValue); // Use the JSON get method to retrieve the value
}