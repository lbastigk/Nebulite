/**
 * @file JsonModifier.hpp
 * @brief Utility functions for modifying JSON keys.
 *        JSON(key) -> value | modifier on value | modifier on value ... -> new value
 */

#ifndef NEBULITE_UTILITY_JSON_MODIFIER_HPP
#define NEBULITE_UTILITY_JSON_MODIFIER_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <span>
#include <string>
#include <vector>

// Nebulite
#include "Interaction/Execution/FuncTree.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Utility {
class JSON;
}

//------------------------------------------
namespace Nebulite::Utility {
class JsonModifier {
private:
    /**
     * @brief The modifier tree is used to apply modifications to JSON values during getting
     *        if the key includes the pipe '|' character, we apply the modifiers in sequence.
     *        Example: get<int>('MyKey.subKey|strLength|add 1')
     *                 will get the length of the string at MyKey.subKey and add 1 to it.
     *        Takes in a JSON* as argument to modify.
     *        Returns true on success, false on failure.
     */
    std::unique_ptr<Interaction::Execution::FuncTree<bool, JSON*>> modifierFuncTree;

    //------------------------------------------
    // Functions

    /**
     * @brief Adds a value to the current JSON value.
     * @param args Additional arguments, none expected for this modifier.
     * @param jsonDoc The JSON document to modify.
     * @return true on success, false on failure.
     */
    bool add(std::span<std::string const> const& args, JSON* jsonDoc);

public:
    JsonModifier();

    /**
     * @brief The key used to store and retrieve the value being modified.
     */
    static std::string const valueKey;

    /**
     * @brief Parses and applies JSON modifiers from the given arguments.
     *        Example: get<int>("MyKey|add 5|multiply 2")
     *        Calls: parse(["add 5", "multiply 2"], inputDocument)
     *        Input document: {"value": 10}
     *        After applying the modifier, the document will be: {"value": 30}
     * @param args A list of arguments representing the modifiers to apply.
     * @param jsonDoc The JSON document to modify. Should hold the value from the get operation
     *                in a specified key. On success, the modified value will be stored back in the same key.
     * @return true if the modifiers were successfully applied, false otherwise.
     */
    bool parse(std::vector<std::string> const& args, JSON* jsonDoc);
};

}   // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_JSON_MODIFIER_HPP