#pragma once
#include "JSON.h"

class VirtualDouble {
    Nebulite::JSON* json_pointer = nullptr;
    std::string key;
    mutable double cache = 0;  // mutable so we can modify it in const methods

public:
    VirtualDouble(Nebulite::JSON* j, const std::string& k) 
        : json_pointer(j), key(k) {}

    // Override dereference operator
    double operator*() const {
        if (json_pointer != nullptr) {
            cache = json_pointer->get<double>(key.c_str(), 0);
        }
        else{
            cache = 0.0;
        }
        return cache;
    }

    // Override arrow operator (if needed)
    double* operator->() const {
        if (json_pointer != nullptr) {
            cache = json_pointer->get<double>(key.c_str(), 0);
        }
        else{
            cache = 0.0;
        }
        return &cache;
    }

    // For getting address (like &mc)
    double* ptr() const {
        if (json_pointer != nullptr) {
            cache = json_pointer->get<double>(key.c_str(), 0);
        } else {
            cache = 0.0;
        }
        return &cache;
    }
};