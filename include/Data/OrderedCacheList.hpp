/**
 * @file OrderedCacheList.hpp
 * @brief Defines a list of double pointers for interaction logic.
 */

#ifndef NEBULITE_DATA_ORDERED_DOUBLE_POINTERS_HPP
#define NEBULITE_DATA_ORDERED_DOUBLE_POINTERS_HPP

//------------------------------------------
// Includes

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Data/Document/ScopedKey.hpp"
#include "Utility/Coordination/SharedMutex.hpp"
#include "Utility/Coordination/IdGenerator.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------

namespace Nebulite::Data {
/**
 * @class Nebulite::Data::DynamicFixedArray
 * @brief Dynamic fixed-size array for double pointers.
 * @details Size is set once at construction and never changes.
 * @todo Change to a simple std::vector or inlined vector later, should be more efficient and simpler.
 */
class DynamicFixedArray {
public:
    DynamicFixedArray() : data_(nullptr), size_(0), capacity_(0) {}

    explicit DynamicFixedArray(size_t const& fixed_size)
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

    [[nodiscard]] double*& at(size_t const& index) const noexcept { return data_[index]; }

    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    [[nodiscard]] double** data() const noexcept { return data_; }

private:
    double** data_;
    size_t size_;
    size_t capacity_;
};

/**
 * @brief Lightweight container for ordered double pointers in expression evaluations.
 */
class OrderedCacheList {
public:
    OrderedCacheList() = default;

    explicit OrderedCacheList(size_t const& exact_size) : orderedValues(exact_size) {}

    DynamicFixedArray orderedValues {};
};

// Vector alias for easier usage of ordered double pointer vectors
using odpvec = DynamicFixedArray;

/**
 * @class MappedOrderedCacheList
 * @brief A thread-safe map from strings to OrderedCacheList objects.
 */
class MappedOrderedCacheList {
public:
    explicit MappedOrderedCacheList(JsonScope& ownerReference)
        : reference(ownerReference) {
    }

    /**
     * @brief Generates a unique ID for an expression based on its string representation.
     * @param identifier The string representation of the expression for which to generate a unique ID.
     * @return A unique ID corresponding to the given expression string. This ID can be used for caching purposes in the MappedOrderedCacheList.
     */
    static size_t generateUniqueId(std::string_view const& identifier) {
        static auto generator = Utility::Coordination::IdGenerator::stringToRollingIdGenerator();
        return generator(identifier);
    }

    /**
     * @brief Ensures the existence of an ordered cache list of double pointers for a set of keys.
     * @param uniqueId The unique ID for the ordered cache list.
     * @param keys The vector of keys to populate the cache with.
     * @return A pointer to the ordered vector of double pointers for the specified keys.
     */
    odpvec* ensureOrderedCacheList(uint64_t const& uniqueId, std::vector<ScopedKeyView> const& keys);

    /**
     * @brief Ensures the existence of an ordered cache list of double pointers for a set of keys. Non-locking version.
     * @param uniqueId The unique ID for the ordered cache list.
     * @param keys The vector of keys to populate the cache with.
     * @return A pointer to the ordered vector of double pointers for the specified keys.
     */
    odpvec* ensureOrderedCacheListNoLock(uint64_t const& uniqueId, std::vector<ScopedKeyView> const& keys);

private:
    /**
     * @brief Reference to the JSON document that owns this cache.
     */
    JsonScope& reference;

    /**
     * @brief Map from unique IDs to OrderedCacheList objects.
     */
    absl::flat_hash_map<uint64_t, OrderedCacheList> map;

    /**
     * @brief Mutex for thread-safe access to the map.
     */
    Utility::Coordination::SharedMutex mtxMap;

    /**
     * @brief Retrieves or creates an ordered cache list of double pointers for a set of keys from the map.
     * @details Not protected by any mutex
     * @param uniqueId The unique id of the entry
     * @param keys The keys to potentially create the entry with
     * @return An ordered vector of double pointers corresponding to the keys, either retrieved from the map or newly created if it did not exist.
     */
    odpvec* fromMap(uint64_t const& uniqueId, std::vector<ScopedKeyView> const& keys);
};
} // namespace Nebulite::Data

#endif // NEBULITE_DATA_ORDERED_DOUBLE_POINTERS_HPP
