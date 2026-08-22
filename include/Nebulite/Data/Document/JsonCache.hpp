#ifndef NEBULITE_DATA_DOCUMENT_JSONCACHE_HPP
#define NEBULITE_DATA_DOCUMENT_JSONCACHE_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint> // NOLINT
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Nebulite/Data/Document/RjDirectAccess.hpp"

//------------------------------------------
namespace Nebulite::Data {

/**
 * @brief The amount of pre-cached double values per Document.
 */
static auto constexpr cachelineSize = 1024 / sizeof(double);

/**
 * @brief Pre-allocated cacheline for fast double value access.
 * @details Instead of always allocating new double values, we use a pre-allocated cacheline.
 *          This reduces memory fragmentation and improves cache locality.
 */
using CacheLine = std::array<double, cachelineSize>;

/**
 * @struct CacheEntry
 * @brief Represents a cached entry in the JSON document, including its value, state, and stable pointer for double values.
 */
struct CacheEntry {
    /**
     * @brief Standard numeric value used for initializing cache entries and failed variant conversions
     */
    static double constexpr standardNumericValue = 0.0;

    /**
     * @enum EntryState
     * @brief Represents the state of a cached entry in the JSON document.
     *        How it works:
     *        - On reloading a full document, all entries become DELETED.
     *        - If we access a double pointer of a deleted/nonexistent value, we mark the entry as VIRTUAL,
     *          as it's a resurrected entry, but its potentially not the real value due to casting.
     *        - A value becomes DIRTY if it was previously CLEAN, and we notice a change in its double value.
     *        - On flushing, all DIRTY entries become CLEAN again. VIRTUAL entries remain VIRTUAL as they are not flushed.
     *        - Values may be marked DELETED if their parent is modified or deleted.
     */
    enum class EntryState : std::uint8_t {
        clean, // Synchronized with RapidJSON document, real value. NOTE: This may be invalid at any time if double pointer is used elsewhere! This just marks the last known state.
        dirty, // Modified in cache, needs flushing to RapidJSON, real value
        derived, // Deleted/nonexistent entry that was accessed via double pointer
        deleted, // Deleted entry due to deserialization or child invalidation, inner value is invalid
        malformed, // A key that is known to be malformed due to transformations. Used in getStableDoublePointer for integrity.
    };

    //------------------------------------------
    // No copying or moving

    CacheEntry(CacheEntry const&) = delete;
    CacheEntry& operator=(CacheEntry const&) = delete;
    CacheEntry(CacheEntry&&) = delete;
    CacheEntry& operator=(CacheEntry&&) = delete;

    //------------------------------------------
    // Data members

    RjDirectAccess::SimpleValue value = standardNumericValue;
    double lastDoubleValue = standardNumericValue;
    double* stableDoublePointer = nullptr; // Stable pointer to double value
    EntryState state = EntryState::dirty; // Default to dirty: each new entry needs flushing
    bool managedInternalDouble = false; // Whether the stable double pointer is managed internally or externally (from cacheline)

    CacheEntry([[clang::lifetimebound]] CacheLine& cl, std::size_t& index) {
        if (index >= cachelineSize) [[unlikely]] {
            stableDoublePointer = new double(standardNumericValue);
            managedInternalDouble = true;
        }
        else [[likely]] {
            // Assign stable double pointer from cacheline
            stableDoublePointer = &cl[index];
            index++;
            *stableDoublePointer = standardNumericValue;
            managedInternalDouble = false;
        }
    }

    ~CacheEntry() {
        if (managedInternalDouble) {
            delete stableDoublePointer;
        }
    }

    void updateNumericValue();

    void markAsDeleted();

    void setValueClean(RjDirectAccess::SimpleValue const& newValue);

    void setValueDirty(RjDirectAccess::SimpleValue const& newValue);

    template <typename NewType>
    std::optional<NewType> convertTo();
};

class JsonCache {
    /**
     * @brief Pre-allocated cacheline for fast, cache-friendly access to double values.
     * @details Should stay unique_ptr to ensure the addresses never change.
     *          With a normal array, the would change on a move, which would invalidate all stable double pointers!
     */
    std::unique_ptr<CacheLine> cacheLine;

    /**
     * @brief Current index in the cacheline for the next double value.
     */
    std::size_t cacheLineIndex = 0;

    /**
     * @brief The Caching system used for fast access to frequently used values.
     */
    absl::flat_hash_map<std::string, std::shared_ptr<CacheEntry>> cache;

    using KeyAndEntry = std::pair<std::string, std::shared_ptr<CacheEntry>>;

    /**
     * @brief Iteration-friendly vector of CacheEntries
     */
    std::vector<KeyAndEntry> cacheVector;

    /**
     * @brief Creates a new cache entry for the given key and returns a reference to it.
     * @param key The key for which to create a new cache entry.
     * @return A reference to the newly created cache entry.
     */
    auto& createNewCacheEntry(std::string_view key) {
        auto newEntry = std::make_shared<CacheEntry>(*cacheLine, cacheLineIndex);
        cache[key] = newEntry;
        cacheVector.emplace_back(std::string(key), newEntry);
        return *newEntry.get();
    }

public:
    JsonCache();

    JsonCache(JsonCache const&) = delete;
    JsonCache& operator=(JsonCache const&) = delete;

    JsonCache(JsonCache&& other) noexcept ;
    JsonCache& operator=(JsonCache&& other) noexcept ;

    ~JsonCache();

    /**
     * @brief Clears the cache, removing all entries and resetting the cache vector.
     * @details This function should only be called when the JSON document is being destroyed
     *          or when it is guaranteed that no stable double pointers are in use.
     * @todo Using Utility::Promise would be nice, but we cannot reference JSON from here due to circular dependency issues.
     *       This is not a big issue as a promise does nothing and is only a programming convenience, but it would be nice to have.
     */
    void clear();

    /**
     * @brief Deletes a cache entry by its key.
     * @param key The key of the cache entry to delete.
     */
    void deleteEntry(std::string_view key);

    /**
     * @brief Returns an iterator to the beginning of the cache vector.
     * @return An iterator to the beginning of the cache vector.
     */
    [[nodiscard]] auto begin() const [[clang::lifetimebound]] -> decltype(cacheVector.begin());

    /**
     * @brief Returns an iterator to the end of the cache vector.
     * @return An iterator to the end of the cache vector.
     */
    [[nodiscard]] auto end() const [[clang::lifetimebound]] -> decltype(cacheVector.end()) ;

    /**
     * @brief Finds a cache entry by its key.
     * @param key The key of the cache entry to find.
     * @return An optional reference to the cache entry if found, or std::nullopt if not found.
     */
    [[nodiscard]] std::optional<CacheEntry&> find(std::string_view key) const [[clang::lifetimebound]] ;

    /**
     * @brief Inserts a new cache entry with the given key and applies a modifier function to it.
     * @tparam F The type of the modifier function, which must be invocable with a CacheEntry& and return void.
     * @param key The key of the cache entry to insert.
     * @param modifier The modifier function to apply to the new cache entry.
     */
    template<typename F>
    void insert(std::string_view key, F&& modifier) {
        static_assert(std::is_invocable_r_v<void, F, CacheEntry&>, "Modifier function must be invocable with CacheEntry& and return void.");
        assert(!find(key).has_value() && "Key already exists in cache.");

        auto& newEntry = createNewCacheEntry(key);
        std::invoke(std::forward<F>(modifier), newEntry);
    }

    /**
     * @brief Inserts a new cache entry with the given key, applies a modifier function to it, and returns a value using a return function.
     * @tparam R The type of the value to return.
     * @tparam F The type of the modifier function, which must be invocable with a CacheEntry& and return void.
     * @tparam RF The type of the return function, which must be invocable with a CacheEntry& and return R.
     * @param key The key of the cache entry to insert.
     * @param modifier The modifier function to apply to the new cache entry.
     * @param returnFunc The return function to use for retrieving the value.
     * @return The value returned by the return function.
     */
    template<typename R, typename F, typename RF>
    R insertAndGet(std::string_view key, F&& modifier, RF&& returnFunc) {
        static_assert(std::is_invocable_r_v<void, F, CacheEntry&>, "Modifier function must be invocable with CacheEntry& and return void.");
        static_assert(std::is_invocable_r_v<R, RF, CacheEntry&>, "Result function must be invocable with CacheEntry& and return R.");
        assert(!find(key).has_value() && "Key already exists in cache.");

        auto& newEntry = createNewCacheEntry(key);
        std::invoke(std::forward<F>(modifier), newEntry);
        auto r = std::invoke(std::forward<RF>(returnFunc), newEntry);
        return r;
    }
};

} // namespace Nebulite::Data
#include "Nebulite/Data/Document/JsonCache.tpp" // NOLINT(misc-include-cleaner)
#endif // NEBULITE_DATA_DOCUMENT_JSONCACHE_HPP
