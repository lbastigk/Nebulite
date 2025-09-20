/**
 * @file DocumentCache.hpp
 * 
 * @brief This file contains the DocumentCache class for managing cached, read-only documents.
 */

#pragma once

//------------------------------------------
// Includes

// Nebulite
#include "Utility/JSON.hpp"
#include "Utility/FileManagement.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
namespace Nebulite {
namespace Utility {
/**
 * @class Nebulite::Utility::DocumentCache
 * @brief Caches documents as read-only for quick access of resources.
 * 
 * This class provides methods to retrieve data from cached documents efficiently.
 * It allows for quick access to frequently used data without the need to reload documents.
 * It also manages the loading of documents as needed.
 * 
 * @todo Unloading of documents is currently not supported.
 * 
 * The basic idea is to add a timekeeper for all loaded documents, tracking their last usage.
 * This, however, results in a O(n) complexity, slowing down the retrieval of documents as the only place to update the last used time is during retrieval.
 * 
 * The better approach is to do a random check, one document at a time each retrieval.
 * If the document is found to be unused for a certain period, it can be unloaded from memory.
 *
 * This is possible, as the exact time of unloading is unimportant.
 *
 * Furthermore, we might wish to set a minimum threshold for how many documents must be loaded before we start unloading them.
 * This would prevent us from unloading documents too aggressively and potentially impacting performance.
 */
class DocumentCache{
public:
    /**
     * @brief Default constructor for DocumentCache.
     */
    DocumentCache() = default;

    /**
     * @brief Retrieves data from a cached document.
     * 
     * Used to quickly access frequently used data without reloading the document. Unloaded documents are automatically loaded and cached.
     * 
     * @tparam T The type of the data to retrieve.
     * 
     * @param doc_key The key identifying the document and the specific data to retrieve: 
     * `<linkToDocument>:<key>`
     * Example:
     * `./Resources/Data/myData.jsonc:key1.key2`
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
    double* getDoublePointerOf(const std::string& doc_key);
private:
    /**
     * @brief Time in milliseconds after which unused documents are unloaded.
     * 
     * Documents that have not been accessed within this time frame will be removed from the cache to free up memory.
     */
    uint64_t unloadAfter_ms = 5 * 60 * 1000; // Unload documents after 5 minutes of inactivity

    /**
     * @brief Updates the cache by checking a random document for its last usage time.
     */
    void update();

    // Cache for read-only documents
    struct ReadOnlyDoc {
        Nebulite::Utility::JSON document;
        Nebulite::Utility::TimeKeeper lastUsed;
    };
    absl::flat_hash_map<std::string, ReadOnlyDoc> ReadOnlyDocs;

    ReadOnlyDoc* getDocument(const std::string& doc);

    // Default value for double pointers, if the document or key is not found
    double zero = 0.0;
};
} // namespace Utility
} // namespace Nebulite

//------------------------------------------
// Definitions of template functions

template  <typename T> 
T Nebulite::Utility::DocumentCache::getData(std::string doc_key, const T& defaultValue) {
    // Split the input into document name and key
    size_t pos = doc_key.find(':');
    if (pos == std::string::npos) {
        return defaultValue; // Return default value if format is incorrect
    }
    std::string doc = doc_key.substr(0, pos);
    std::string key = doc_key.substr(pos + 1);

    Nebulite::Utility::DocumentCache::ReadOnlyDoc* docPtr = getDocument(doc);

    // Check if the document exists in the cache
    if (docPtr == nullptr) {
        return defaultValue; // Return default value if document loading fails
    }

    // Retrieve the value from the document
    T data = docPtr->document.get<T>(key.c_str(), defaultValue);

    // Update the cache (unload old documents)
    update();

    // Return key:
    return data; // Use the JSON get method to retrieve the value
}