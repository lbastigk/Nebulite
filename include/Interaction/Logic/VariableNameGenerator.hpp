#ifndef NEBULITE_INTERACTION_LOGIC_VARIABLE_NAME_GENERATOR_HPP
#define NEBULITE_INTERACTION_LOGIC_VARIABLE_NAME_GENERATOR_HPP

//------------------------------------------
// Includes

// Standard library

#include <string>
#include <absl/container/flat_hash_map.h>

//------------------------------------------
namespace Nebulite::Interaction::Logic {

/**
 * @class VariableNameGenerator
 * @brief Generates unique variable names based on a base name.
 * @note We may wish to retrieve variable names for debugging purposes later on.
 *       For now, after the expression is parsed, the mapping is discarded.
 */
class VariableNameGenerator {
    absl::flat_hash_map<std::string, std::string> variableNameToIdMap;

    /**
     * @brief Converts a given number to a string representation using a custom base-26 encoding.
     * @details The encoding uses lowercase letters 'a' to 'z' to represent values from 0 to 25.
     *          For numbers greater than 25, it continues with combinations of letters (e.g., 26 becomes "za", 27 becomes "zb", etc.).
     * @param number The number to convert to a string representation.
     * @return A string representation of the given number using the custom base-26 encoding.
     */
    static std::string numberToString(uint16_t number);

public:
    /**
     * @brief Gets a unique variable name for a given base name.
     * @param baseName The base name for which to generate a unique variable name.
     * @return A unique variable name corresponding to the base name.
     */
    std::string getUniqueName(std::string const& baseName);

    /**
     * @brief Clears the variable name mapping, resetting the generator to its initial state.
     */
    void clear();
};
} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_VARIABLE_NAME_GENERATOR_HPP
