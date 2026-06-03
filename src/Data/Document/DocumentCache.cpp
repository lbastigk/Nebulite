//------------------------------------------
// Includes

// Standard library
#include <cstddef> // NOLINT
#include <string>
#include <string_view>

// Nebulite
#include "Data/Document/DocumentCache.hpp"
#include "Data/Document/KeyType.hpp"
#include "Data/Document/ReadOnlyDocs.hpp"

//------------------------------------------
namespace Nebulite::Data {

// Basic value retrieval: type,size,serial, etc.

double const* DocumentCache::getStableDoublePointer(std::string const& doc_key) const {
    return getValueFromCache<double const*>(doc_key, &zero, [](ReadOnlyDoc const* docPtr, std::string_view const& key) {
        return docPtr->document.getStableDoublePointer(key);
    });
}

KeyType DocumentCache::memberType(std::string const& doc_key) const {
    return getValueFromCache<KeyType>(doc_key, KeyType::null, [](ReadOnlyDoc const* docPtr, std::string_view const& key) {
        return docPtr->document.memberType(key);
    });
}

size_t DocumentCache::memberSize(std::string const& doc_key) const {
    return getValueFromCache<size_t>(doc_key, 0, [](ReadOnlyDoc const* docPtr, std::string_view const& key) {
        return docPtr->document.memberSize(key);
    });
}

std::string DocumentCache::serialize(std::string const& doc_key) const {
    return getValueFromCache<std::string>(doc_key, "{}", [](ReadOnlyDoc const* docPtr, std::string_view const& key) {
        if (key.empty()) {
            return docPtr->serial;
        }
        JSON const subDoc = docPtr->document.getSubDoc(key);
        return subDoc.serialize();
    });
}

// Document serialization

JSON DocumentCache::getSubDoc(std::string const& doc_key) const {
    auto [doc, key] = splitDocKey(doc_key);

    ReadOnlyDoc const* docPtr = readOnlyDocs.getDocument(doc);
    if (!docPtr) {
        return JSON{};
    }

    // Check if the document exists in the cache
    JSON data = docPtr->document.getSubDoc(key);

    // Update the cache (unload old documents) and return the size
    readOnlyDocs.update();
    return data;
}

std::string DocumentCache::getDocString(std::string_view const& link) const {
    ReadOnlyDoc const* docPtr = readOnlyDocs.getDocument(link);

    // Check if the document exists in the cache
    if (docPtr == nullptr) {
        return JSON().serialize(); // Return empty JSON if document loading fails
    }

    // Return string of document:
    std::string serial = docPtr->serial;

    // Update the cache (unload old documents)
    readOnlyDocs.update();

    return serial;
}

} // namespace Nebulite::Data
