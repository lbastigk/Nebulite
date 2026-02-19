#include "Interaction/Logic/VariableNameGenerator.hpp"

namespace Nebulite::Interaction::Logic {

std::string VariableNameGenerator::numberToString(uint16_t number) {
    std::string result;
    do {
        char const ch = static_cast<char>('a' + number % 26);
        result = ch + result;
        number /= 26;
        if (number > 0) {
            number -= 1; // Adjust for 0-based indexing
        }
    } while (number > 0);
    return result;
}

std::string VariableNameGenerator::getUniqueName(std::string const& baseName) {
    // Check if the base name already exists in the map
    if (variableNameToIdMap.find(baseName) != variableNameToIdMap.end()) {
        return variableNameToIdMap[baseName];
    }
    // Generate a new unique name
    std::string uniqueName = numberToString(static_cast<uint16_t>(variableNameToIdMap.size()));
    variableNameToIdMap[baseName] = uniqueName;
    return uniqueName;
}

void VariableNameGenerator::clear() {
    variableNameToIdMap.clear();
}

} // namespace Nebulite::Interaction::Logic
