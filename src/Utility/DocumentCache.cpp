#include "Utility/DocumentCache.hpp"

void Nebulite::Utility::DocumentCache::update() {
    readOnlyDocs.update();
}

double* Nebulite::Utility::DocumentCache::getDoublePointerOf(const std::string& doc_key) {
    // Split the input into document name and key
    size_t pos = doc_key.find(':');
    if (pos == std::string::npos) {
        return &zero; // Return a pointer to zero if format is incorrect
    }
    std::string doc = doc_key.substr(0, pos);
    std::string key = doc_key.substr(pos + 1);

    ReadOnlyDoc* docPtr = readOnlyDocs.getDocument(doc);
    if (docPtr == nullptr) {
        return &zero; // Return a pointer to zero if document loading fails
    }

    // Update the cache (unload old documents)
    update();

    // Return pointer to double value inside the document
    return docPtr->document.getDoublePointerOf(key);
}