/**
 * @file DocumentCache.hpp
 * @brief This file contains the DocumentCache class for managing cached, read-only documents.
 */

#ifndef NEBULITE_DATA_DOCUMENTCACHE_HPP
#define NEBULITE_DATA_DOCUMENTCACHE_HPP

//------------------------------------------
// Includes

// Standard library
#include <stdexcept>

// Nebulite
#include "ReadOnlyDocs.hpp"

//------------------------------------------
namespace Nebulite::Data {

/**
 * @class Nebulite::Data::DocumentCache
 * @brief Provides methods to retrieve data from cached documents efficiently.
 *        It allows for quick access to frequently used data without the need to reload documents.
 *        It also manages the loading/unloading of documents as needed.
 */
class DocumentCache {
public:
    /**
     * @brief Default constructor for DocumentCache.
     */
    DocumentCache() = default;

    /**
     * @brief Retrieves data from a cached document.
     *        Used to quickly access frequently used data without reloading the document. Unloaded documents are automatically loaded and cached.
     * @tparam T The type of the data to retrieve.
     * @param doc_key The key identifying the document and the specific data to retrieve: 
     *                `<linkToDocument>:<key>`
     *                Example:
     *                `./Resources/Data/myData.jsonc:key1.key2`
     * @param defaultValue The value to return if the document or key is not found.
     * @return The retrieved data or the default value as type T.
     */
    template <typename T>
    T get(std::string const& doc_key, T const& defaultValue = T());

    /**
     * @brief Gets a sub-document from the JSON document.
     *        This function retrieves a sub-document from the JSON document.
     *        If the key does not exist, an empty JSON object is returned.
     *        Note that the document is flushed for data integrity.
     * @param doc_key The link and key of the sub-document to retrieve.
     * @return The sub-document associated with the key, or an empty JSON object if the key does not exist.
     */
    Nebulite::Data::JSON getSubDoc(std::string const& doc_key) {
        auto [doc, key] = splitDocKey(doc_key);
        Nebulite::Data::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

        // Check if the document exists in the cache
        if (docPtr == nullptr) {
            return Nebulite::Data::JSON{}; // Return empty JSON if document loading fails
        }

        // Retrieve the sub-document from the document
        Nebulite::Data::JSON data = docPtr->document.getSubDoc(key);

        // Update the cache (unload old documents)
        update();

        // Return key:
        return data; // Use the JSON get method to retrieve the value
    }

    /**
     * @brief Retrieves a pointer to a double value from a cached document.
     *        This is used to compile tinyexpr expressions with direct linkage to in-document values.
     * @param doc_key The key identifying the document and the specific data to retrieve: <linkToDocument>:<key>
     * @return A pointer to the retrieved double value. 
     *         Guaranteed to be valid even if the key does not exist within the document,
     *         or if the document itself is not found!
     */
    double* getStableDoublePointer(std::string const& doc_key);

    /**
     * @brief Checks the type of any key in the JSON document.
     *        If the key does not exist, the type is considered null.
     * @param doc_key The document and its key to check.
     * @return The type of the key.
     */
    Nebulite::Data::JSON::KeyType memberType(std::string const& doc_key) {
        auto [doc, key] = splitDocKey(doc_key);

        Nebulite::Data::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

        // Check if the document exists in the cache
        if (docPtr == nullptr) {
            return Nebulite::Data::JSON::KeyType::null; // Return null if document loading fails
        }

        // Retrieve the key type from the document
        Nebulite::Data::JSON::KeyType const type = docPtr->document.memberType(key);

        // Update the cache (unload old documents)
        update();

        // Return key:
        return type; // Use the JSON get method to retrieve the value
    }

    /**
     * @brief Checks the size of a key in the JSON document.
     *        If the key does not exist, the size is considered 0.
     *        If the key represents a document, the size is also considered 1.
     * @param doc_key The key to check.
     * @return The size of the key.
     */
    size_t memberSize(std::string const& doc_key) {
        auto [doc, key] = splitDocKey(doc_key);

        Nebulite::Data::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

        // Check if the document exists in the cache
        if (docPtr == nullptr) {
            return 0; // Return 0 if document loading fails
        }

        // Retrieve the key size from the document
        size_t const size = docPtr->document.memberSize(key);

        // Update the cache (unload old documents)
        update();

        // Return key:
        return size; // Use the JSON get method to retrieve the value
    }

    /**
     * @brief Serializes the entire document or a portion of the document
     * @param doc_key The document and key to serialize.
     * @return The serialized JSON string.
     */
    std::string serialize(std::string const& doc_key) {
        auto [doc, key] = splitDocKey(doc_key);

        Nebulite::Data::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

        // Check if the document exists in the cache
        if (docPtr == nullptr) {
            return "{}"; // Return empty JSON object if document loading fails
        }

        // if key is empty, return entire document
        std::string data;
        if (key.empty()) {
            data = docPtr->serial;
        }
        else {
            // Retrieve the sub-document from the document
            Nebulite::Data::JSON subDoc = docPtr->document.getSubDoc(key);
            data = subDoc.serialize();
        }
        update();
        return data;
    }

    /**
     * @brief Retrieves the entire document as a serialized string.
     * @param link The link to the document.
     * @return The serialized JSON string of the entire document.
     */
    std::string getDocString(std::string const& link) {
        Nebulite::Data::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(link);

        // Check if the document exists in the cache
        if (docPtr == nullptr) {
            return Nebulite::Data::JSON().serialize(); // Return empty JSON if document loading fails
        }

        // Return string of document:
        std::string serial = docPtr->document.serialize();

        // Update the cache (unload old documents)
        update();

        return serial;
    }

private:
    /**
     * @brief Read-only document cache.
     */
    Nebulite::Data::ReadOnlyDocs readOnlyDocs;

    // Default value for double pointers, if the document or key is not found
    /**
     * @brief Default zero value for stable double pointers.
     * @todo Currently used as a fallback for missing documents/keys and set to 0 on each access.
     *       Consider optimizing this by implementing a more sophisticated handling mechanism.
     *       Perhaps storing a unique double pointer for each missing key instead and keeping track of them and their keys.
     *       This would usually not be needed, as zero is only assigned for missing documents?
     *       But at least one pointer per key would be better than resetting this one each time.
     */
    double zero = 0.0;

    /**
     * @brief Splits a doc:key string into its components
     */
    static std::pair<std::string, std::string> splitDocKey(std::string const& doc_key) {
        size_t const pos = doc_key.find(':');
        if (pos == std::string::npos) {
            // No colon found, meaning the entire string is document name/link
            return {doc_key, ""};
        }
        std::string const doc = doc_key.substr(0, pos);
        std::string const key = doc_key.substr(pos + 1);
        return {doc, key};
    }

    /**
     * @brief Updates the cache by checking a random document for its last usage time.
     */
    void update();
};
} // namespace Nebulite::Data

//------------------------------------------
// Definitions of template functions

template <typename T>
T Nebulite::Data::DocumentCache::get(std::string const& doc_key, T const& defaultValue) {
    auto [doc, key] = splitDocKey(doc_key);

    Nebulite::Data::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

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

#endif // NEBULITE_DATA_DOCUMENTCACHE_HPP