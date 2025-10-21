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
        zero = new double(0.0);
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
    T get(std::string doc_key, const T& defaultValue = T());

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
    Nebulite::Utility::JSON get_subdoc(const std::string& doc_key){
        auto [doc, key] = splitDocKey(doc_key);

        Nebulite::Utility::DocumentCache::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

        // Check if the document exists in the cache
        if (docPtr == nullptr) {
            return Nebulite::Utility::JSON(global); // Return empty JSON if document loading fails
        }

        // Retrieve the sub-document from the document
        Nebulite::Utility::JSON data = docPtr->document.get_subdoc(key.c_str());

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
    double* get_stable_double_ptr(const std::string& doc_key);

    /**
     * @brief Checks the type of a key in the JSON document.
     * 
     * This function checks the type of a key in the JSON document.
     * If the key does not exist, the type is considered null.
     * 
     * @param doc_key The document and its key to check.
     * @return The type of the key.
     */
    Nebulite::Utility::JSON::KeyType memberCheck(std::string doc_key){
        auto [doc, key] = splitDocKey(doc_key);

        Nebulite::Utility::DocumentCache::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

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
    uint32_t memberSize(std::string doc_key){
        auto [doc, key] = splitDocKey(doc_key);

        Nebulite::Utility::DocumentCache::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

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
    std::string serialize(std::string doc_key) {
        auto [doc, key] = splitDocKey(doc_key);

        Nebulite::Utility::DocumentCache::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

        // Check if the document exists in the cache
        if (docPtr == nullptr) {
            return "{}"; // Return empty JSON object if document loading fails
        }

        // Retrieve the sub-document from the document
        Nebulite::Utility::JSON data = docPtr->document.get_subdoc(key.c_str());

        // Update the cache (unload old documents)
        update();

        // Return key:
        return data.serialize(); // Use the JSON get method to retrieve the value
    }

    std::string getDocString(std::string link){
        Nebulite::Utility::DocumentCache::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(link);

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
     * @brief Splits a doc:key string into its components
     */
    std::pair<std::string, std::string> splitDocKey(const std::string& doc_key){
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

    /**
     * @struct ReadOnlyDoc
     * @brief Represents a read-only document with its associated metadata.
     */
    struct ReadOnlyDoc {
        Nebulite::Utility::JSON document; // The actual JSON document
        Nebulite::Utility::TimeKeeper lastUsed;

        ReadOnlyDoc(Nebulite::Core::GlobalSpace* globalSpace) : document(globalSpace), lastUsed() {}
    };

    /**
     * @brief Map of document paths to their corresponding ReadOnlyDoc instances.
     * 
     * Takes care of loading and unloading documents as needed.
     */
    struct ReadOnlyDocs{
        private:
            /**
             * @brief Time in milliseconds after which unused documents are unloaded.
             * 
             * Documents that have not been accessed within this time frame will be removed from the cache to free up memory.
             */
            uint64_t unloadAfter_ms = 5 * 60 * 1000; // Unload documents after 5 minutes of inactivity

            /**
             * @brief Contains the cached documents mapped by their file paths.
             */
            absl::flat_hash_map<std::string, ReadOnlyDoc> docs;

            /**
             * @brief Reference to the global space for document creation.
             */
            Nebulite::Core::GlobalSpace* globalSpace;

        public:
            /**
             * @brief Constructor that takes a GlobalSpace pointer.
             */
            ReadOnlyDocs(Nebulite::Core::GlobalSpace* global) : globalSpace(global) {
                // Validate that globalSpace is not null
                if (globalSpace == nullptr) {
                    throw std::invalid_argument("DocumentCache: GlobalSpace pointer cannot be null");
                }
            }
            /**
             * @brief Updates the cache by checking a random document for its last usage time,
             * and unloading it if it has been unused for too long.
             */
            void update(){
                if(docs.empty()) {
                    return; // No documents to check
                }

                // Check the last used time of a random document
                auto it = docs.begin();
                std::advance(it, rand() % docs.size());
                ReadOnlyDoc* docPtr = &it->second;

                // If the document has not been used recently, unload it
                if (docPtr->lastUsed.projected_dt() > unloadAfter_ms) {
                    docs.erase(it);
                }
            }

            /**
             * @brief Proper retrieval of a document, loading it if not already cached.
             * And updating its metadata.
             */
            ReadOnlyDoc* getDocument(const std::string& doc) {
                // Validate inputs and state
                if (doc.empty()) {
                    return nullptr;
                }
                if (globalSpace == nullptr) {
                    return nullptr;
                }
                
                // Check if the document exists in the cache
                auto it = docs.find(doc);
                if (it == docs.end()) {
                    // Load the document if it doesn't exist
                    std::string serial = Nebulite::Utility::FileManagement::LoadFile(doc);
                    if (serial.empty()) {
                        return nullptr; // Return nullptr if document loading fails
                    }
                    auto result = docs.emplace(doc, ReadOnlyDoc(globalSpace));
                    if (!result.second) {
                        // Emplace failed for some reason
                        return nullptr;
                    }
                    result.first->second.document.deserialize(serial);
                    it = result.first;
                }
                ReadOnlyDoc* docPtr = &it->second;
                docPtr->lastUsed.update();
                return docPtr;
            }
    }readOnlyDocs;

    // Default value for double pointers, if the document or key is not found
    double* zero;
};
} // namespace Utility
} // namespace Nebulite

//------------------------------------------
// Definitions of template functions

template  <typename T> 
T Nebulite::Utility::DocumentCache::get(std::string doc_key, const T& defaultValue) {
    auto [doc, key] = splitDocKey(doc_key);

    Nebulite::Utility::DocumentCache::ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);

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


