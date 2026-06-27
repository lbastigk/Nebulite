#ifndef DATA_DOCUMENT_DOCUMENTCACHE_HPP
#define DATA_DOCUMENT_DOCUMENTCACHE_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>

// Nebulite
#include "Data/Document/ReadOnlyDocs.hpp"
#include "Interaction/Context.hpp"

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
     * @return The retrieved data or an error.
     */
    template <typename T>
    std::expected<T, SimpleValueRetrievalError> get(std::string const& doc_key) const ;

    /**
     * @brief Retrieves a pointer to a double value from a cached document.
     *        This is used to compile tinyexpr expressions with direct linkage to in-document values.
     * @param doc_key The key identifying the document and the specific data to retrieve: <linkToDocument>:<key>
     * @return A pointer to the retrieved double value. 
     *         Guaranteed to be valid even if the key does not exist within the document,
     *         or if the document itself is not found!
     */
    double const* getStableDoublePointer(std::string const& doc_key) const ;

    /**
     * @brief Checks the type of any key in the JSON document.
     *        If the key does not exist, the type is considered null.
     * @param doc_key The document and its key to check.
     * @return The type of the key.
     */
    KeyType memberType(std::string const& doc_key) const ;

    /**
     * @brief Checks the size of a key in the JSON document.
     *        If the key does not exist, the size is considered 0.
     *        If the key represents a document, the size is also considered 1.
     * @param doc_key The key to check.
     * @return The size of the key.
     */
    std::size_t memberSize(std::string const& doc_key) const ;

    /**
     * @brief Serializes the entire document or a portion of the document
     * @param doc_key The document and key to serialize.
     * @return The serialized JSON string.
     */
    std::string serialize(std::string const& doc_key) const ;

    /**
     * @brief Gets a sub-document from the JSON document.
     *        This function retrieves a sub-document from the JSON document.
     *        If the key does not exist, an empty JSON object is returned.
     *        Note that the document is flushed for data integrity.
     * @param doc_key The link and key of the sub-document to retrieve.
     * @return The sub-document associated with the key, or an empty JSON object if the key does not exist.
     */
    JSON getSubDoc(std::string const& doc_key) const ;

    /**
     * @brief Retrieves the entire document as a serialized string.
     * @param link The link to the document.
     * @return The serialized JSON string of the entire document.
     */
    std::string getDocString(std::string_view const& link) const ;

private:
    /**
     * @brief Read-only document cache.
     */
    ReadOnlyDocs readOnlyDocs;

    /**
     * @brief Default zero value for stable double pointer retrieval, if document access fails.
     */
    static double constexpr zero = 0.0;

    /**
     * @brief Splits a doc:key string into its components, also works for doc|transform or doc:key|transform
     * @todo Returning a pair of string_views should be possible?
     */
    static std::pair<std::string, std::string> splitDocKey(std::string const& doc_key) {
        std::string_view doc_key_view(doc_key);
        Utility::StringHandler::strip(doc_key_view, ' '); // Remove whitespace for more forgiving input handling

        auto const barPos = doc_key_view.find(JSON::SpecialCharacter::transformationPipe);
        auto const colonPos = doc_key_view.find(Interaction::ContextDeriver::contextKeySeparator);

        // Choose the first occurring separator
        auto const pos = std::min(colonPos, barPos);

        if (pos == std::string::npos) {
            // No colon found, meaning the entire string is document name/link
            return {std::string(doc_key_view), ""};
        }
        auto const doc = doc_key_view.substr(0, pos);
        auto const key = doc_key_view.substr(pos + 1);

        // Add back the transform part if needed
        if (pos == barPos) {
            return {std::string(doc), JSON::SpecialCharacter::transformationPipe + std::string(key)};
        }
        return {std::string(doc), std::string(key)};
    }

    /**
     * @brief Templated helper function to retrieve a value from the read only cache
     */
    template <typename ValueType>
    ValueType getValueFromCache(std::string const& doc_key, ValueType const& defaultValue, std::function<ValueType(ReadOnlyDoc const* doc, std::string_view const& key)> const& retrievalFunction) const {
        static_assert(!std::is_same_v<ValueType, JSON>, "JSON values cannot be used here. Please re-implement the retrieval logic in a custom way instead of using this helper function.");

        auto [doc, key] = splitDocKey(doc_key);

        ReadOnlyDoc const* docPtr = readOnlyDocs.getDocument(doc);
        if (!docPtr) {
            return defaultValue;
        }

        // Check if the document exists in the cache
        ValueType const data = retrievalFunction(docPtr, key);

        // Update the cache (unload old documents) and return the size
        readOnlyDocs.update();
        return data;
    }
};

//------------------------------------------
// Definitions of template functions

template <typename T>
std::expected<T, SimpleValueRetrievalError> DocumentCache::get(std::string const& doc_key) const {
    auto [doc, key] = splitDocKey(doc_key);

    ReadOnlyDoc const* docPtr = readOnlyDocs.getDocument(doc);

    // Check if the document exists in the cache
    if (docPtr == nullptr) {
        // Use get on an empty JSON so we can still apply transformations
        // This way, important transformation commands like assert aren't overlooked just because the document is missing, which would make debugging very difficult
        thread_local JSON const emptyJson;
        return emptyJson.get<T>(key);
    }

    // Retrieve the value from the document
    auto data = docPtr->document.get<T>(key);

    // Update the cache (unload old documents)
    readOnlyDocs.update();

    // Return key:
    return data; // Use the JSON get method to retrieve the value
}

} // namespace Nebulite::Data
#endif // DATA_DOCUMENT_DOCUMENTCACHE_HPP
