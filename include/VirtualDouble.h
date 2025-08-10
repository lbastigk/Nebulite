#pragma once
#include "JSON.h"
#include "DocumentCache.h"

namespace Nebulite{
class VirtualDouble {
    Nebulite::DocumentCache* documentCache = nullptr;
    Nebulite::JSON** json_dual_pointer = nullptr;
    std::string key;
    mutable double cache = 0;  // mutable so we can modify it in const methods

public:
    VirtualDouble(Nebulite::JSON** j, const std::string& k, Nebulite::DocumentCache* documentCache) 
        : json_dual_pointer(j), key(k), documentCache(documentCache) {
            // Removing self/other/global prefixes
            if (key.find("self.") == 0) key = key.substr(5);
            else if (key.find("other.") == 0) key = key.substr(6);
            else if (key.find("global.") == 0) key = key.substr(7);
        }


    // OLD:
    /*
    // Override dereference operator
    double operator*() const {
        std::cout << "Dereferencing VirtualDouble for key: " << key << std::endl;

        if (json_dual_pointer != nullptr && *json_dual_pointer != nullptr) {
            cache = (*json_dual_pointer)->get<double>(key.c_str(), 0);
        }
        else if (documentCache != nullptr) {
            cache = documentCache->getData<double>(key.c_str(), 0);
        }
        return cache;
    }

    // Override arrow operator (if needed)
    double* operator->() const {
        std::cout << "Dereferencing VirtualDouble for key: " << key << std::endl;

        if (json_dual_pointer != nullptr && *json_dual_pointer != nullptr) {
            cache = (*json_dual_pointer)->get<double>(key.c_str(), 0);
        }
        else if (documentCache != nullptr) {
            cache = documentCache->getData<double>(key.c_str(), 0);
        }
        return &cache;
    }
    */

    double* ptr() const {
        if (json_dual_pointer != nullptr && *json_dual_pointer != nullptr) {
            cache = (*json_dual_pointer)->get<double>(key.c_str(), 0);
        }
        else if (documentCache != nullptr)  {
            cache = documentCache->getData<double>(key.c_str(), 0);
        }
        return &cache;
    }
};
}