#include "Utility/DocumentCache.hpp"

void Nebulite::Utility::DocumentCache::update() {
    readOnlyDocs.update();
}

double* Nebulite::Utility::DocumentCache::getStableDoublePointer(std::string const& doc_key) {
    auto [doc, key] = splitDocKey(doc_key);

    ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);
    if (docPtr == nullptr) {
        // Return a pointer to zero if document loading fails
        zero = 0.0;     // Make sure zero is always 0.0
        return &zero;
    }

    // Update the cache (unload old documents)
    update();

    // Return pointer to double value inside the document
    return docPtr->document.getStableDoublePointer(key);
}