/**
 * @file Branch.hpp
 * @brief Contains the declaration of the Branch class used for generating
 *        branch-structured containers.
 */

#ifndef NEBULITE_DATA_BRANCH_HPP
#define NEBULITE_DATA_BRANCH_HPP

//------------------------------------------
// Includes

// Standard library
#include <concepts>
#include <cstddef>
#include <mutex>
#include <new> // for placement new
#include <random>
#include <shared_mutex>
#include <stdexcept>
#include <type_traits>
#include <vector>

// External
#include "absl/container/inlined_vector.h"

//------------------------------------------

namespace Nebulite::Data {

/**
 * @class Branch
 * @brief A template class for creating branch-structured containers.
 *        Provides thread-safe access and dynamic resizing based on IDs.
 * @details The idea is to allow access to elements via IDs, where each layer
 *          of the branch can have its own logic for mapping IDs to indices.
 * @tparam StoreType The type of elements to be stored in the branch.
 * @tparam idType The type of the ID used to access elements.
 * @tparam MaxBits The maximum number of bits for the branch (default is 8).
 */
template<typename StoreType, typename idType, std::size_t MaxBits = 8>
class Branch {
public:
    // Check MaxBits validity
    static_assert(MaxBits > 0 && MaxBits < (sizeof(std::size_t) * 8),
                  "MaxBits must be > 0 and fit in size_t");

    // Ensure StoreType
    // - provides apply() method
    // - is default-constructible
    static_assert(requires(StoreType& s) { s.apply(); },
                          "StoreType must provide apply() method");
    static_assert(std::is_default_constructible_v<StoreType>,
                  "StoreType must be default-constructible for cleanup()");

    //------------------------------------------
    // Constructor / Destructor

    Branch() = default;
    virtual ~Branch() = default;

    //------------------------------------------
    // Copy / Move semantics

    // Copy constructor: create a new mutex, copy protected by locking source
    Branch(Branch const& other) {
        std::scoped_lock lock(other.storageMutex);
        storage = other.storage;
        randNum = other.randNum;
        for (size_t i = 0; i < trackerSize; ++i) {
            tracker[i].store(other.tracker[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
    }

    // Move constructor: create a new mutex, move-protect by locking source
    Branch(Branch&& other) noexcept {
        std::scoped_lock lock(other.storageMutex);
        storage = std::move(other.storage);
        randNum = other.randNum;
        for (size_t i = 0; i < trackerSize; ++i) {
            tracker[i].store(other.tracker[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
    }

    // Copy assignment: lock both (scoped_lock uses std::lock internally), keep this->mutex intact
    Branch& operator=(Branch const& other) {
        if (this == &other) return *this;
        std::scoped_lock lock(storageMutex, other.storageMutex);
        storage = other.storage;
        randNum = other.randNum;
        for (size_t i = 0; i < trackerSize; ++i) {
            tracker[i].store(other.tracker[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
        return *this;
    }

    // Move assignment: lock both, move storage, keep this->mutex intact
    Branch& operator=(Branch&& other) noexcept {
        if (this == &other) return *this;
        std::scoped_lock lock(storageMutex, other.storageMutex);
        storage = std::move(other.storage);
        randNum = other.randNum;
        for (size_t i = 0; i < trackerSize; ++i) {
            tracker[i].store(other.tracker[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
        return *this;
    }

    //------------------------------------------
    // Public Methods

    /**
     * @brief Accesses the element corresponding to the given ID.
     * @param id The ID of the element to access.
     * @return A unique pointer to the element corresponding to the given ID.
     * @throws std::out_of_range if the computed index exceeds MaxSize.
     */
    std::shared_ptr<StoreType> at(idType const& id);

    /**
     * @brief Probabilistic cleanup of inactive entries.
     */
    void cleanup();

    /**
     * @brief Applies a function to all active entries in the tree.
     */
    void apply();

protected:
    /**
     * @brief Converts an ID to its corresponding index in the storage.
     * @param id The ID to convert.
     * @return The index corresponding to the given ID.
     */
    [[nodiscard]] virtual size_t idToIndex(idType const& id) const = 0;

private:
    static constexpr std::size_t MaxSize = std::size_t(1) << MaxBits;

    //------------------------------------------
    // Storage

    // Storage array
    template<typename T> using arr = absl::InlinedVector<T, 4>;
    arr<std::shared_ptr<StoreType>> storage;

    // Mutex for thread-safe access
    mutable std::shared_mutex storageMutex;

    // RNG generator for cleanup
    std::mt19937 randNum{std::random_device{}()};

    //------------------------------------------
    // Access Tracking

    static constexpr std::size_t trackerSize = (MaxSize + 63) / 64;
    std::array<std::atomic<uint64_t>, trackerSize> tracker{0};

    void markAccessed(std::size_t index) {
        std::size_t arrayIndex = index / 64;
        std::size_t bitPosition = index % 64;
        uint64_t mask = uint64_t(1) << bitPosition;
        tracker[arrayIndex].fetch_or(mask, std::memory_order_relaxed);
    }

    bool wasAccessed(std::size_t index) const {
        std::size_t arrayIndex = index / 64;
        std::size_t bitPosition = index % 64;
        uint64_t mask = uint64_t(1) << bitPosition;
        return (tracker[arrayIndex].load(std::memory_order_relaxed) & mask) != 0;
    }

    void resetAccessed() {
        for (auto& atomicVal : tracker) {
            atomicVal.store(0, std::memory_order_relaxed);
        }
    }
};

} // namespace Nebulite::Data
#include "Data/Branch.tpp"
#endif // NEBULITE_DATA_BRANCH_HPP
