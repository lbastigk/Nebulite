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
#include <random>
#include <stdexcept>
#include <type_traits>
#include <vector>

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
    // - provides isActive() -> bool
    // - is default-constructible
    static_assert(requires(StoreType const& s) { { s.isActive() } -> std::convertible_to<bool>; },
              "StoreType must provide isActive() -> bool");
    static_assert(std::is_default_constructible_v<StoreType>,
                  "StoreType must be default-constructible for cleanup()");

    Branch() = default;
    virtual ~Branch() = default;

    /**
     * @brief Accesses the element corresponding to the given ID.
     * @param id The ID of the element to access.
     * @return A reference to the element corresponding to the given ID.
     * @throws std::out_of_range if the computed index exceeds MaxSize.
     */
    StoreType& at(idType const& id) {
        std::size_t const index = idToIndex(id);
        if (index >= MaxSize) {
            throw std::out_of_range(std::string(__FUNCTION__) + " - index exceeds MaxSize");
        }

        std::scoped_lock lock(mutex);
        if (index >= storage.size()) {
            storage.resize(index + 1);
        }
        return storage[index];
    }

    /**
     * @brief Probabilistic cleanup of inactive entries.
     */
    void cleanup() {
        // Take random index and check if inactive
        std::uniform_int_distribution<size_t> distribution(0, MaxSize - 1);
        size_t const index = distribution(randNum);

        std::scoped_lock lock(mutex);
        if (index < storage.size() && !storage[index].isActive()) {
            storage[index] = StoreType(); // Reset to default
        }
    }

    /**
     * @brief Checks if the container is active
     * @return true if the container has any elements, false otherwise.
     */
    [[nodiscard]] bool isActive() const {
        bool active = false;
        std::scoped_lock lock(mutex);
        for (size_t i = 0; i < size(); ++i) {
            if (storage[i].isActive()) {
                active = true;
                break;
            }
        }
        return active;
    }

protected:
    /**
     * @brief Converts an ID to its corresponding index in the storage.
     * @param id The ID to convert.
     * @return The index corresponding to the given ID.
     */
    [[nodiscard]] virtual size_t idToIndex(idType const& id) const = 0;

private:
    [[nodiscard]] size_t size() const {
        std::scoped_lock lock(mutex);
        return storage.size();
    }

    static constexpr std::size_t MaxSize = std::size_t(1) << MaxBits;

    // Mutex for thread-safe access
    mutable std::mutex mutex;

    // Storage for the branch elements
    std::vector<StoreType> storage;

    // RNG generator
    std::mt19937 randNum{std::random_device{}()};
};

} // namespace Nebulite::Data
#endif // NEBULITE_DATA_BRANCH_HPP
