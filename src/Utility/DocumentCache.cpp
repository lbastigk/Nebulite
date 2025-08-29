#include "Utility/DocumentCache.h"

double* Nebulite::Utility::DocumentCache::getDoublePointerOf(const std::string& doc_key) {
    // Split the input into document name and key
    size_t pos = doc_key.find(':');
    if (pos == std::string::npos) {
        return &zero; // Return a pointer to zero if format is incorrect
    }
    std::string doc = doc_key.substr(0, pos);
    std::string key = doc_key.substr(pos + 1);

    if(ReadOnlyDocs.find(doc) == ReadOnlyDocs.end()) {
        // Load the document if it doesn't exist
        std::string serial = Nebulite::Utility::FileManagement::LoadFile(doc);
        if (serial.empty()) {
            return &zero;
        }
        ReadOnlyDocs[doc].deserialize(serial);
    }

    // Register the external cache
    if (ReadOnlyDocs.find(doc) != ReadOnlyDocs.end()) {
        return ReadOnlyDocs[doc].getDoublePointerOf(key);
    }
    return &zero; // Return a pointer to zero if the document or key is not found
}