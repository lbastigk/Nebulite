/**
 * @file OrderedDoublePointers.hpp
 * @brief Defines a list of double pointers for interaction logic.
 */

#pragma once

#include <vector>
#include <mutex>

#include "Nebulite.hpp"

#include "absl/container/flat_hash_map.h"

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
 * @struct MappedOrderedPointers
 * @brief A thread-safe map from strings to OrderedDoublePointers objects.
 */
struct MappedOrderedPointers{
    absl::flat_hash_map<std::string, OrderedDoublePointers> map;
    std::mutex mtx;
};
} // namespace Nebulite::Utility

