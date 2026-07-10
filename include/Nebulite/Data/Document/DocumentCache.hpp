#ifndef NEBULITE_DATA_DOCUMENT_DOCUMENTCACHE_HPP
#define NEBULITE_DATA_DOCUMENT_DOCUMENTCACHE_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <expected>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

// Nebulite
#include "Nebulite/Data/Document/JSON.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Data/Document/ReadOnlyDocs.hpp"
#include "Nebulite/Data/Document/SimpleValueError.hpp"

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
    std::string getDocString(std::string_view link) const ;

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
    static std::pair<std::string, std::string> splitDocKey(std::string const& doc_key);

    /**
     * @brief Templated helper function to retrieve a value from the read only cache
     */
    template <typename ValueType>
    ValueType getValueFromCache(std::string const& doc_key, ValueType const& defaultValue, std::function<ValueType(ReadOnlyDoc const* doc, std::string_view key)> const& retrievalFunction) const ;
};
} // namespace Nebulite::Data
#include "Nebulite/Data/Document/DocumentCache.tpp" // NOLINT
#endif // NEBULITE_DATA_DOCUMENT_DOCUMENTCACHE_HPP
