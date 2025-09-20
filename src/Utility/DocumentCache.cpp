#include "Utility/DocumentCache.hpp"

void Nebulite::Utility::DocumentCache::update() {
    // Randomly check one document for its last used time
    if (ReadOnlyDocs.empty()) {
        return; // No documents to check
    }

    // Check the last used time of a random document
    auto it = ReadOnlyDocs.begin();
    std::advance(it, rand() % ReadOnlyDocs.size());
    ReadOnlyDoc* docPtr = &it->second;

    // If the document has not been used recently, unload it
    if (docPtr->lastUsed.projected_dt() > unloadAfter_ms) {
        ReadOnlyDocs.erase(it);
    }
}

Nebulite::Utility::DocumentCache::ReadOnlyDoc* Nebulite::Utility::DocumentCache::getDocument(const std::string& doc){
    // Check if the document exists in the cache
    if (ReadOnlyDocs.find(doc) == ReadOnlyDocs.end()) {
        // Load the document if it doesn't exist
        std::string serial = Nebulite::Utility::FileManagement::LoadFile(doc);
        if (serial.empty()) {
            return nullptr; // Return nullptr if document loading fails
        }
        ReadOnlyDocs[doc].document.deserialize(serial);
    }
    ReadOnlyDoc* docPtr = &ReadOnlyDocs[doc];
    docPtr->lastUsed.update();
    return docPtr;
}

double* Nebulite::Utility::DocumentCache::getDoublePointerOf(const std::string& doc_key) {
    // Split the input into document name and key
    size_t pos = doc_key.find(':');
    if (pos == std::string::npos) {
        return &zero; // Return a pointer to zero if format is incorrect
    }
    std::string doc = doc_key.substr(0, pos);
    std::string key = doc_key.substr(pos + 1);

    ReadOnlyDoc* docPtr = getDocument(doc);
    if (docPtr == nullptr) {
        return &zero; // Return a pointer to zero if document loading fails
    }

    // Update the cache (unload old documents)
    update();

    // Return pointer to double value inside the document
    return docPtr->document.getDoublePointerOf(key);
}