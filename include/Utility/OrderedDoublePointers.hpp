/**
 * @file OrderedDoublePointers.hpp
 * @brief Defines a list of double pointers for interaction logic.
 */

#pragma once

//------------------------------------------
// Includes

// Standard library
#include <vector>
#include <mutex>

// External
#include "absl/container/flat_hash_map.h"

// Nebulite
#include "Nebulite.hpp"

//------------------------------------------
// Defines

#define ORDERED_DOUBLE_POINTERS_QUICKCACHE_SIZE 30

//------------------------------------------

namespace Nebulite::Utility {
/**
 * @class OrderedDoublePointers
 * @brief A list of double pointers used in expression evaluations.
 * 
 * Allows for strict ordering of double pointers, potentially reducing overhead of get_double_ptr calls,
 * if the same order is used multiple times.
 */
class OrderedDoublePointers {
public:
    OrderedDoublePointers() = default;

    std::vector<double*> values;
};

/**
 * @struct MappedOrderedDoublePointers
 * @brief A thread-safe map from strings to OrderedDoublePointers objects.
 */
template <typename hashtype>
struct MappedOrderedDoublePointers{
    absl::flat_hash_map<hashtype, OrderedDoublePointers> map;
    std::mutex mtx;

    /**
     * @brief Quick cache for the first few OrderedDoublePointers entries.
     * 
     * This array allows for fast access to frequently used entries without the overhead of a hashmap lookup.
     * 
     * @todo In order for this to work in production, we need a global functioncall that generates unique IDs for expressions.
     * E.g. if we know our engine relies a lot on expression A, but it might not be used first,
     * its best to then call this function early on to assign it a low unique ID.
     */
    OrderedDoublePointers quickCache[ORDERED_DOUBLE_POINTERS_QUICKCACHE_SIZE];
};
} // namespace Nebulite::Utility

