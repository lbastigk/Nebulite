/**
 * @file OrderedDoublePointers.hpp
 * @brief Defines a list of double pointers for interaction logic.
 */

#ifndef NEBULITE_DATA_ORDERED_DOUBLE_POINTERS_HPP
#define NEBULITE_DATA_ORDERED_DOUBLE_POINTERS_HPP

//------------------------------------------
// Includes

// Standard library
#include <shared_mutex>

// External
#include <absl/container/flat_hash_map.h>

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScopeBase;
} // namespace Nebulite::Data

namespace Nebulite::Interaction::Logic {
class VirtualDouble;
} // namespace Nebulite::Interaction::Logic

//------------------------------------------

namespace Nebulite::Data {
/**
 * @class Nebulite::Data::DynamicFixedArray
 * @brief Dynamic fixed-size array for double pointers.
 * @details Size is set once at construction and never changes.
 */
class DynamicFixedArray {
public:
    DynamicFixedArray() : data_(nullptr), size_(0), capacity_(0) {}

    explicit DynamicFixedArray(size_t fixed_size)
        : data_(fixed_size > 0 ? new double*[fixed_size] : nullptr),
          size_(0),
          capacity_(fixed_size) {
    }

    // Move constructor
    DynamicFixedArray(DynamicFixedArray&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Move assignment
    DynamicFixedArray& operator=(DynamicFixedArray&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    ~DynamicFixedArray() {
        delete[] data_;
    }

    // No copy operations since size is fixed
    DynamicFixedArray(DynamicFixedArray const&) = delete;
    DynamicFixedArray& operator=(DynamicFixedArray const&) = delete;

    void push_back(double* ptr) {
        if (size_ < capacity_) {
            data_[size_++] = ptr;
        }
    }

    double*& at(size_t index) const noexcept { return data_[index]; }

    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    double** data() const noexcept { return data_; }

private:
    double** data_;
    size_t size_;
    size_t capacity_;
};

/**
 * @brief Lightweight container for ordered double pointers in expression evaluations.
 */
class OrderedDoublePointers {
public:
    OrderedDoublePointers() = default;

    explicit OrderedDoublePointers(size_t exact_size) : orderedValues(exact_size) {
    }

    DynamicFixedArray orderedValues {};
};

// Vector alias for easier usage of ordered double pointer vectors
using odpvec = Nebulite::Data::DynamicFixedArray;

/**
 * @class MappedOrderedDoublePointers
 * @brief A thread-safe map from strings to OrderedDoublePointers objects.
 */
class MappedOrderedDoublePointers {
public:
    explicit MappedOrderedDoublePointers(JsonScopeBase& ownerReference)
        : reference(ownerReference) {
    }

    /**
     * @brief Size of the quickcache for ordered double pointers.
     *
     * This defines how many OrderedDoublePointers can be cached for quick access
     * without needing to look them up in a hashmap.
     * see MappedOrderedDoublePointers::quickCache for important considerations.
     */
    static constexpr size_t quickCacheSize = 30;

    /**
     * @brief Ensures the existence of an ordered cache list of double pointers for "other" context variables.
     *        Checks if the current "other" reference JSON document contains a cached, ordered list of double pointers
     *        corresponding to all variables referenced by this Expression in the "other" context. If the cache entry does not exist,
     *        it is created and populated for fast indexed access during expression evaluation.
     * @param uniqueId The unique ID of the expression.
     * @param contextOther The vector of virtual doubles in the "other" context to populate the cache with.
     * @return A pointer to the ordered vector of double pointers for the referenced "other" variables.
     */
    odpvec* ensureOrderedCacheList(
        uint64_t uniqueId,
        std::vector<std::shared_ptr<Interaction::Logic::VirtualDouble>> const& contextOther
        );

    /**
     * @brief Ensures the existence of an ordered cache list of double pointers for a set of keys.
     * @param uniqueId The unique ID for the ordered cache list.
     * @param keys The vector of keys to populate the cache with.
     * @return A pointer to the ordered vector of double pointers for the specified keys.
     */
    odpvec* ensureOrderedCacheList(
        uint64_t const& uniqueId,
        std::vector<std::string_view> const& keys
        );

private:
    /**
     * @brief Reference to the JSON document that owns this cache.
     */
    JsonScopeBase& reference;

    /**
     * @brief Map from unique IDs to OrderedDoublePointers objects.
     */
    absl::flat_hash_map<uint64_t, OrderedDoublePointers> map;

    /**
     * @brief Mutex for thread-safe access to the cache.
     */
    std::shared_mutex mtxCache;

    /**
     * @brief Mutex for thread-safe access to the map.
     */
    std::shared_mutex mtxMap;

    /**
     * @brief Quick cache for the first few OrderedDoublePointers entries.
     * 
     * This array allows for fast access to frequently used entries without the overhead of a hashmap lookup.
     * 
     * @todo In order for this to work in production, we need a global functioncall that generates unique IDs for expressions.
     *       E.g. if we know our engine relies a lot on expression A, but it might not be used first,
     *       its best to then call this function early on to assign it a low unique ID.
     *       Idea: DomainModule for performance: GSDM_Performance.hpp that has a function to register frequently used expressions:
     *       registerId <string>
     *       This function assigns a unique integer ID to the string expression, which can then be used to access the quick cache.
     *       This function should be called right at the start of the program for known expressions to ensure they get low IDs.
     */
    OrderedDoublePointers quickCache[quickCacheSize];
};
} // namespace Nebulite::Data

#endif // NEBULITE_DATA_ORDERED_DOUBLE_POINTERS_HPP
