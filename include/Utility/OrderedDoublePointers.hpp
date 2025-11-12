/**
 * @file OrderedDoublePointers.hpp
 * @brief Defines a list of double pointers for interaction logic.
 */

#ifndef NEBULITE_UTILITY_ORDEREDDOUBLEPOINTERS_HPP
#define NEBULITE_UTILITY_ORDEREDDOUBLEPOINTERS_HPP

//------------------------------------------
// Includes

// Standard library
#include <mutex>

// External
#include <absl/container/flat_hash_map.h>

//------------------------------------------

namespace Nebulite::Utility {
/**
 * @brief Dynamic fixed-size array for double pointers.
 * Size is set once at construction and never changes.
 */
class DynamicFixedArray {
public:
    DynamicFixedArray() : data_(nullptr), size_(0), capacity_(0){}
    
    explicit DynamicFixedArray(size_t fixed_size) 
        : data_(fixed_size > 0 ? new double*[fixed_size] : nullptr), 
          size_(0), 
          capacity_(fixed_size){}

    // Move constructor
    DynamicFixedArray(DynamicFixedArray&& other) noexcept 
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_){
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Move assignment
    DynamicFixedArray& operator=(DynamicFixedArray&& other) noexcept {
        if (this != &other){
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

    ~DynamicFixedArray(){
        delete[] data_;
    }

    // No copy operations since size is fixed
    DynamicFixedArray(DynamicFixedArray const&) = delete;
    DynamicFixedArray& operator=(DynamicFixedArray const&) = delete;

    void push_back(double* ptr){
        if (size_ < capacity_){
            data_[size_++] = ptr;
        }
    }

    double*& at(size_t index) noexcept { return data_[index]; }
    
    bool empty() const noexcept { return size_ == 0; }

    double** data() noexcept { return data_; }

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
    explicit OrderedDoublePointers(size_t exact_size) : orderedValues(exact_size){}
    DynamicFixedArray orderedValues;
};

/**
 * @class MappedOrderedDoublePointers
 * @brief A thread-safe map from strings to OrderedDoublePointers objects.
 */
class MappedOrderedDoublePointers{
public:
    /**
     * @brief Size of the quickcache for ordered double pointers.
     * This defines how many OrderedDoublePointers can be cached for quick access
     * without needing to look them up in a hashmap.
     * 
     * see MappedOrderedDoublePointers::quickCache for important considerations.
     */
    static constexpr size_t quickCacheSize = 30;

    /**
     * @brief Map from unique IDs to OrderedDoublePointers objects.
     */
    absl::flat_hash_map<uint64_t, OrderedDoublePointers> map;

    /**
     * @brief Mutex for thread-safe access to the map.
     */
    std::mutex mtx;

    /**
     * @brief Quick cache for the first few OrderedDoublePointers entries.
     * 
     * This array allows for fast access to frequently used entries without the overhead of a hashmap lookup.
     * 
     * @todo In order for this to work in production, we need a global functioncall that generates unique IDs for expressions.
     * E.g. if we know our engine relies a lot on expression A, but it might not be used first,
     * its best to then call this function early on to assign it a low unique ID.
     * 
     * Idea: DomainModule for performance: GSDM_Performance.hpp that has a function to register frequently used expressions:
     * registerId <string>
     * This function assigns a unique integer ID to the string expression, which can then be used to access the quick cache.
     * This function should be called right at the start of the program for known expressions to ensure they get low IDs.
     */
    OrderedDoublePointers quickCache[quickCacheSize];
};
} // namespace Nebulite::Utility

// Vector alias for easier usage of ordered double pointer vectors
using odpvec = Nebulite::Utility::DynamicFixedArray;

#endif // NEBULITE_UTILITY_ORDEREDDOUBLEPOINTERS_HPP

