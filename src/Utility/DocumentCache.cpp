#include "Utility/DocumentCache.hpp"

void Nebulite::Utility::DocumentCache::update() {
    readOnlyDocs.update();
}

double* Nebulite::Utility::DocumentCache::get_stable_double_ptr(const std::string& doc_key) {
    auto [doc, key] = splitDocKey(doc_key);

    ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);
    if (docPtr == nullptr) {
        return zero; // Return a shared_ptr to zero if document loading fails
    }

    // Update the cache (unload old documents)
    update();

    // Return pointer to double value inside the document
    return docPtr->document.get_stable_double_ptr(key);
}