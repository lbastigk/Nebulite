#ifndef NEBULITE_UTILITY_RANDOM_HPP
#define NEBULITE_UTILITY_RANDOM_HPP

//------------------------------------------
// Includes

// Standard library
#include <functional>
#include <string>

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class Random
 * @brief Simple RNG class using string seeds.
 * @tparam RngSize The type used for RNG values (e.g., std::uint32_t, std::uint64_t).
 */
template<typename RngSize>
class Random {
public:
    /**
     * @brief Retrieves the current RNG value.
     */
    RngSize get() noexcept {
        return current;
    }

    /**
     * @brief Retrieves the last RNG value.
     */
    void update(std::string const& seed) noexcept {
        last = current;
        current = static_cast<RngSize>(rngHasher(seed));
    }

    /**
     * @brief Rolls back to the last RNG value. 
     */
    void rollback() noexcept {
        current = last;
    }

private:
    /**
     * @brief Hasher for generating RNG values from a string.
     */
    std::hash<std::string> rngHasher;

    /**
     * @brief Current RNG value.
     */
    RngSize current = 0;

    /**
     * @brief Last RNG value.
     */
    RngSize last = 0;
};
} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_RANDOM_HPP
