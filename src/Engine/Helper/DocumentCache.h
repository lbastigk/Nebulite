#pragma once

#include "JSON.h"
#include "Time.h"


class DocumentCache{
    template  <typename T> 
    T getKey(std::string doc,std::string key);

    void updateDocumentStorageCounters();
private:
    // TODO: 
    const uint64_t MAX_LAST_USED = 600000;   // 600s
    struct DocumentStorage{
        Nebulite::JSON doc;             // Doc with read-only info
        uint32_t timeLastUsed = 0;      // Counter when doc was last used
    };
    absl::flat_hash_map<std::string,Nebulite::JSON> ReadOnlyDocs;
};