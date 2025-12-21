#ifndef NEBULITE_INTERACTION_LOGIC_VARIABLE_NAME_GENERATOR_HPP
#define NEBULITE_INTERACTION_LOGIC_VARIABLE_NAME_GENERATOR_HPP

//------------------------------------------
// Includes

// Standard library

#include <string>
#include <cstdint>
#include <absl/container/flat_hash_map.h>

namespace Nebulite::Interaction::Logic {

/**
 * @class VariableNameGenerator
 * @brief Generates unique variable names based on a base name.
 * @note We may wish to retrieve variable names for debugging purposes later on.
 *       For now, after the expression is parsed, the mapping is discarded.
 */
class VariableNameGenerator {
private:
    absl::flat_hash_map<std::string, std::string> variableNameToIdMap;

    // Converts a number to a string
    // 0 -> "a"
    // 25 -> "z"
    // 26 -> "za"
    // etc...
    static std::string numberToString(uint16_t number) {
        std::string result;
        do {
            char ch = static_cast<char>('a' + (number % 26));
            result = ch + result;
            number /= 26;
            if (number > 0) {
                number -= 1; // Adjust for 0-based indexing
            }
        } while (number > 0);
        return result;
    }

public:
    std::string getUniqueName(std::string const& baseName) {
        // Check if the base name already exists in the map
        if (variableNameToIdMap.find(baseName) != variableNameToIdMap.end()) {
            return variableNameToIdMap[baseName];
        } else {
            // Generate a new unique name
            std::string uniqueName = numberToString(static_cast<uint16_t>(variableNameToIdMap.size()));
            variableNameToIdMap[baseName] = uniqueName;
            return uniqueName;
        }
    }

    void clear() {
        variableNameToIdMap.clear();
    }
};
} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_VARIABLE_NAME_GENERATOR_HPP