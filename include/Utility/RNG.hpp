/**
 * @file RNG.hpp
 * @brief Random Number Generation utilities.
 */

#ifndef NEBULITE_UTILITY_RNG_HPP
#define NEBULITE_UTILITY_RNG_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>

//------------------------------------------

namespace Nebulite::Utility {
/**
 * @class RNG
 * @brief Simple RNG class using string seeds.
 * 
 * @tparam rngSize_t The type used for RNG values (e.g., uint32_t, uint64_t).
 */
template<typename rngSize_t>
class RNG {
public:
    /**
     * @brief Retrieves the current RNG value.
     */
    rngSize_t get(){
        return current;
    }

    /**
     * @brief Retrieves the last RNG value.
     */
    void update(std::string const& seed){
        last = current;
        current = static_cast<rngSize_t>(rng_hasher(seed));
    }

    /**
     * @brief Rolls back to the last RNG value. 
     */
    void rollback(){
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
    rngSize_t current = 0;

    /**
     * @brief Last RNG value.
     */
    rngSize_t last = 0;
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_RNG_HPP
