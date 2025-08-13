#pragma once

#include "JSON.h"
#include "Time.h"
#include "FileManagement.h"

namespace Nebulite {
class DocumentCache{
public:
    DocumentCache() = default;

    template  <typename T> 
    T getData(std::string doc_key, const T& defaultValue = T());

    double* getDoublePointerOf(const std::string& doc_key) {
        // Split the input into document name and key
        size_t pos = doc_key.find(':');
        if (pos == std::string::npos) {
            return nullptr; // Return nullptr if format is incorrect
        }
        std::string doc = doc_key.substr(0, pos);
        std::string key = doc_key.substr(pos + 1);

        if(ReadOnlyDocs.find(doc) == ReadOnlyDocs.end()) {
            // Load the document if it doesn't exist
            std::string serial = FileManagement::LoadFile(doc);
            if (serial.empty()) {
                return nullptr;
            }
            ReadOnlyDocs[doc].deserialize(serial);
        }

        // Register the external cache
        if (ReadOnlyDocs.find(doc) != ReadOnlyDocs.end()) {
            return ReadOnlyDocs[doc].getDoublePointerOf(key);
        }
        return nullptr;
    }
private:
    absl::flat_hash_map<std::string,Nebulite::JSON> ReadOnlyDocs;
};
}

// Expected input: ./Resources/Data/myData.jsonc:key1.key2
template  <typename T> 
T Nebulite::DocumentCache::getData(std::string doc_key, const T& defaultValue) {
    // Split the input into document name and key
    size_t pos = doc_key.find(':');
    if (pos == std::string::npos) {
        return defaultValue; // Return default value if format is incorrect
    }
    std::string doc = doc_key.substr(0, pos);
    std::string key = doc_key.substr(pos + 1);

    // Check if the document exists in the cache
    if (ReadOnlyDocs.find(doc) == ReadOnlyDocs.end()) {
        // Load the document if it doesn't exist
        std::string serial = FileManagement::LoadFile(doc);
        if (serial.empty()) {
            return defaultValue; // Return default value if document loading fails
        }
        ReadOnlyDocs[doc].deserialize(serial);
    }

    // Return key:
    return ReadOnlyDocs[doc].get<T>(key.c_str(), defaultValue); // Use the JSON get method to retrieve the value
}
