#ifndef NEBULITE_UTILITY_RNG_HPP
#define NEBULITE_UTILITY_RNG_HPP

//------------------------------------------
// Includes

// Standard library
#include <functional>
#include <string>

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class RNG
 * @brief Simple RNG class using string seeds.
 * @tparam rngSize The type used for RNG values (e.g., std::uint32_t, std::uint64_t).
 */
template<typename rngSize>
class RNG {
public:
    /**
     * @brief Retrieves the current RNG value.
     */
    rngSize get() noexcept {
        return current;
    }

    /**
     * @brief Retrieves the last RNG value.
     */
    void update(std::string const& seed) noexcept {
        last = current;
        current = static_cast<rngSize>(rng_hasher(seed));
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
    std::hash<std::string> rng_hasher;

    /**
     * @brief Current RNG value.
     */
    rngSize current = 0;

    /**
     * @brief Last RNG value.
     */
    rngSize last = 0;
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_RNG_HPP
