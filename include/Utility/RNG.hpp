/**
 * @file RNG.hpp
 * @brief Random Number Generation utilities.
 */

#ifndef NEBULITE_UTILITY_RNG_HPP
#define NEBULITE_UTILITY_RNG_HPP

//------------------------------------------
// Includes

// Standard Library
#include <string>
#include <cstdint>

//------------------------------------------

namespace Nebulite::Utility {

template<typename rng_size_t>
class RNG {
public:
    RNG() = default;
    ~RNG() = default;

    /**
     * @brief Retrieves the current RNG value.
     */
    rng_size_t get(){
        return current;
    }

    /**
     * @brief Retrieves the last RNG value.
     */
    void update(std::string seed){
        last = current;
        current = static_cast<rng_size_t>(rng_hasher(seed));
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

    rng_size_t current = 0;
    rng_size_t last = 0;
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_RNG_HPP