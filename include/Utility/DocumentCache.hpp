/**
 * @file DocumentCache.hpp
 * 
 * @brief This file contains the DocumentCache class for managing cached, read-only documents.
 */

#pragma once

//------------------------------------------
// Includes

// Standard library
#include <stdexcept>

// Nebulite
#include "Utility/ReadOnlyDocs.hpp"

//------------------------------------------
namespace Nebulite::Utility {

/**
 * @class Nebulite::Utility::DocumentCache
 * @brief Caches documents as read-only for quick access of resources.
 * 
 * This class provides methods to retrieve data from cached documents efficiently.
 * It allows for quick access to frequently used data without the need to reload documents.
 * It also manages the loading of documents as needed.
 */
class DocumentCache{
public:
    /**
     * @brief Default constructor for DocumentCache.
     */
    explicit DocumentCache(Nebulite::Core::GlobalSpace* globalSpace) : global(globalSpace), readOnlyDocs(globalSpace) {
        if (globalSpace == nullptr) {
            throw std::invalid_argument("DocumentCache: GlobalSpace pointer cannot be null");
        }
    }

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
    T get(std::string const& doc_key, const T& defaultValue = T());

    /**
     * @brief Gets a sub-document from the JSON document.
     * 
     * This function retrieves a sub-document from the JSON document.
     * If the key does not exist, an empty JSON object is returned.
     * 
     * Note that the document is flushed.
     * 
     * @param doc_key The link and key of the sub-document to retrieve.
     * @return The sub-document associated with the key, or an empty JSON object if the key does not exist.
     */
    Nebulite::Utility::JSON get_subdoc(std::string const& doc_key){
        auto [doc, key] = splitDocKey(doc_key);

        Nebulite::Utility::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

        // Check if the document exists in the cache
        if (docPtr == nullptr) {
            return Nebulite::Utility::JSON(global); // Return empty JSON if document loading fails
        }

        // Retrieve the sub-document from the document
        Nebulite::Utility::JSON data = docPtr->document.get_subdoc(key);

        // Update the cache (unload old documents)
        update();

        // Return key:
        return data; // Use the JSON get method to retrieve the value
    }

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
    double* getStableDoublePointer(std::string const& doc_key);

    /**
     * @brief Checks the type of a key in the JSON document.
     * 
     * This function checks the type of a key in the JSON document.
     * If the key does not exist, the type is considered null.
     * 
     * @param doc_key The document and its key to check.
     * @return The type of the key.
     */
    Nebulite::Utility::JSON::KeyType memberCheck(std::string const& doc_key) {
        auto [doc, key] = splitDocKey(doc_key);

        Nebulite::Utility::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

        // Check if the document exists in the cache
        if (docPtr == nullptr) {
            return Nebulite::Utility::JSON::KeyType::null; // Return null if document loading fails
        }

        // Retrieve the key type from the document
        Nebulite::Utility::JSON::KeyType type = docPtr->document.memberCheck(key);

        // Update the cache (unload old documents)
        update();

        // Return key:
        return type; // Use the JSON get method to retrieve the value
    }

    /**
     * @brief Checks the size of a key in the JSON document.
     * 
     * This function checks the size of a key in the JSON document.
     * 
     * If the key does not exist, the size is considered 0.
     * 
     * If the key represents a document, the size is also considered 1.
     * 
     * @param doc_key The key to check.
     * @return The size of the key.
     */
    uint32_t memberSize(std::string const& doc_key) {
        auto [doc, key] = splitDocKey(doc_key);

        Nebulite::Utility::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

        // Check if the document exists in the cache
        if (docPtr == nullptr) {
            return 0; // Return 0 if document loading fails
        }

        // Retrieve the key size from the document
        uint32_t size = docPtr->document.memberSize(key);

        // Update the cache (unload old documents)
        update();

        // Return key:
        return size; // Use the JSON get method to retrieve the value
    }

    /**
     * @brief Serializes the entire document or a portion of the document
     * 
     * @param doc_key The document and key to serialize.
     * @return The serialized JSON string.
     */
    std::string serialize(std::string const& doc_key) {
        auto [doc, key] = splitDocKey(doc_key);

        Nebulite::Utility::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

        // Check if the document exists in the cache
        if (docPtr == nullptr) {
            return "{}"; // Return empty JSON object if document loading fails
        }

        // Retrieve the sub-document from the document
        Nebulite::Utility::JSON data = docPtr->document.get_subdoc(key);

        // Update the cache (unload old documents)
        update();

        // Return key:
        return data.serialize(); // Use the JSON get method to retrieve the value
    }

    /**
     * @brief Retrieves the entire document as a serialized string.
     * 
     * @param link The link to the document.
     * @return The serialized JSON string of the entire document.
     */
    std::string getDocString(std::string const& link){
        Nebulite::Utility::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(link);

        // Check if the document exists in the cache
        if (docPtr == nullptr) {
            return Nebulite::Utility::JSON(global).serialize(); // Return empty JSON if document loading fails
        }

        // Return string of document:
        std::string serial = docPtr->document.serialize();

        // Update the cache (unload old documents)
        update();

        return serial;
    }

private:
    /**
     * @brief Link to the global space.
     */
    Nebulite::Core::GlobalSpace* global;

    /**
     * @brief Read-only document cache.
     */
    Nebulite::Utility::ReadOnlyDocs readOnlyDocs;

    /**
     * @brief Splits a doc:key string into its components
     */
    std::pair<std::string, std::string> splitDocKey(std::string const& doc_key){
        size_t pos = doc_key.find(':');
        if (pos == std::string::npos) {
            // No colon found, meaning the entire string is document name/link
            return {doc_key, ""};
        }
        std::string doc = doc_key.substr(0, pos);
        std::string key = doc_key.substr(pos + 1);
        return {doc, key};
    }

    /**
     * @brief Updates the cache by checking a random document for its last usage time.
     */
    void update();

    // Default value for double pointers, if the document or key is not found
    double zero = 0.0;
};
} // namespace Nebulite::Utility 

//------------------------------------------
// Definitions of template functions

template  <typename T> 
T Nebulite::Utility::DocumentCache::get(std::string const& doc_key, const T& defaultValue) {
    auto [doc, key] = splitDocKey(doc_key);

    Nebulite::Utility::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

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


