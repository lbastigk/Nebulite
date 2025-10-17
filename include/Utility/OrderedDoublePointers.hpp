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
#include "absl/container/inlined_vector.h"

// Nebulite
#include "Nebulite.hpp"

//------------------------------------------
// Defines

#define ORDERED_DOUBLE_POINTERS_QUICKCACHE_SIZE 30

//------------------------------------------

namespace Nebulite::Utility {
/**
 * @brief Dynamic fixed-size array for double pointers.
 * Size is set once at construction and never changes.
 */
class DynamicFixedArray {
public:
    DynamicFixedArray() : data_(nullptr), size_(0), capacity_(0) {}
    
    explicit DynamicFixedArray(size_t fixed_size) 
        : data_(fixed_size > 0 ? new double*[fixed_size] : nullptr), 
          size_(0), 
          capacity_(fixed_size) {}

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
    DynamicFixedArray(const DynamicFixedArray&) = delete;
    DynamicFixedArray& operator=(const DynamicFixedArray&) = delete;

    void push_back(double* ptr) {
        if (size_ < capacity_) {
            data_[size_++] = ptr;
        }
    }

    /**
     * @brief Reserve space and pre-allocate for exactly the needed size.
     * This is a no-op since space is already allocated, but kept for compatibility.
     */
    void reserve(size_t) { /* No-op: space already allocated */ }

    double*& operator[](size_t index) { return data_[index]; }
    const double* operator[](size_t index) const { return data_[index]; }

    double*& at(size_t index) { return data_[index]; }
    const double* at(size_t index) const { return data_[index]; }

    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }
    void clear() { size_ = 0; }

    double** begin() { return data_; }
    double** end() { return data_ + size_; }
    const double* const* begin() const { return data_; }
    const double* const* end() const { return data_ + size_; }

    double** data() { return data_; }
    const double* const* data() const { return data_; }

private:
    double** data_;
    size_t size_;
    size_t capacity_;
};

/**
 * @brief Fixed-size array wrapper for double pointers with vector-like interface.
 * Template version for compile-time known sizes.
 */
template<size_t N>
class FixedDoubleArray {
public:
    FixedDoubleArray() : size_(0) {}

    void push_back(double* ptr) {
        if (size_ < N) {
            data_[size_++] = ptr;
        }
    }

    double*& operator[](size_t index) { return data_[index]; }
    const double* operator[](size_t index) const { return data_[index]; }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    void clear() { size_ = 0; }

    double** begin() { return data_; }
    double** end() { return data_ + size_; }
    const double* const* begin() const { return data_; }
    const double* const* end() const { return data_ + size_; }

    double** data() { return data_; }
    const double* const* data() const { return data_; }

private:
    double* data_[N];
    size_t size_;
};

/**
 * @class OrderedDoublePointers
 * @brief A list of double pointers used in expression evaluations.
 * 
 * Allows for strict ordering of double pointers, potentially reducing overhead of get_double_ptr calls,
 * if the same order is used multiple times.
 */
class OrderedDoublePointers {
public:
    /**
     * @brief Default constructor - uses fallback container.
     */
    OrderedDoublePointers() : orderedValues() {}

    /**
     * @brief Constructor with exact size for maximum performance.
     * @param exact_size The exact number of elements this container will hold.
     */
    explicit OrderedDoublePointers(size_t exact_size) : orderedValues(exact_size) {}

    /**
     * @brief Maximum inline size for the container.
     */
    static constexpr size_t max_inline_size = 32;

    /**
     * @brief Container for double pointers.
     * 
     * Uses a dynamically-sized fixed array that allocates exactly the needed space
     * once at construction time, providing optimal memory usage and cache locality.
     */
    DynamicFixedArray orderedValues;
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

using odpvec = Nebulite::Utility::DynamicFixedArray;

