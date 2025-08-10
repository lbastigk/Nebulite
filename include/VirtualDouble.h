/*
VirtualDouble is a wrapper class designed to ensure consistent access to a double value for tinyexpr
Each value is linked to a document and updated before evaluation.
This ensures that TinyExpr can compile an expression, while also allowing us to manually update its values
from JSON documents.
*/

#pragma once
#include "JSON.h"
#include "DocumentCache.h"

namespace Nebulite{
class VirtualDouble {
    Nebulite::DocumentCache* documentCache = nullptr;
    std::string key;
    double cache = 0.0;

public:
    VirtualDouble(const std::string& k, Nebulite::DocumentCache* documentCache) 
        : key(k), documentCache(documentCache) {
            // Removing self/other/global prefixes
            if (key.find("self.") == 0) key = key.substr(5);
            else if (key.find("other.") == 0) key = key.substr(6);
            else if (key.find("global.") == 0) key = key.substr(7);
        }

    std::string getKey() {
        return key;
    }

    void updateCache(Nebulite::JSON* json) {
        if (json != nullptr) {
            cache = json->get<double>(key.c_str(), 0);
        }
        else if (documentCache != nullptr) {
            cache = documentCache->getData<double>(key.c_str(), 0);
        }
    }

    double* ptr(){
        return &cache;
    }

    double get() {
        return cache;
    }

};
}