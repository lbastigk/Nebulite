#ifndef NEBULITE_DATA_DOCUMENT_DOCUMENTCACHE_TPP
#define NEBULITE_DATA_DOCUMENT_DOCUMENTCACHE_TPP

//------------------------------------------
// Includes

// Standard library
#include <expected>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

// Nebulite
#include "Nebulite/Data/Document/JSON.hpp"
#include "Nebulite/Data/Document/ReadOnlyDocs.hpp"
#include "Nebulite/Data/Document/SimpleValueError.hpp"

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_DATA_DOCUMENT_DOCUMENTCACHE_HPP
#include "Nebulite/Data/Document/DocumentCache.hpp"
#endif // NEBULITE_DATA_DOCUMENT_DOCUMENTCACHE_HPP

//------------------------------------------
namespace Nebulite::Data {

template <typename ValueType>
ValueType DocumentCache::getValueFromCache(std::string const& doc_key, ValueType const& defaultValue, std::function<ValueType(ReadOnlyDoc const* doc, std::string_view key)> const& retrievalFunction) const {
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
#endif // NEBULITE_DATA_DOCUMENT_DOCUMENTCACHE_TPP
