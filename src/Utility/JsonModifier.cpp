//------------------------------------------
// Includes

#include "Utility/JSON.hpp"
#include "Utility/JsonModifier.hpp"

//------------------------------------------
namespace Nebulite::Utility {

JsonModifier::JsonModifier() {
    modifierFuncTree = std::make_unique<Interaction::Execution::FuncTree<bool, JSON*>>("JSON Modifier FuncTree", true, false);

    //------------------------------------------
    // Bind modifier functions

    // Arithmetic Modifiers
    bindModifierFunction(&JsonModifier::add, addName, &addDesc);
    bindModifierFunction(&JsonModifier::multiply, multiplyName, &multiplyDesc);
    bindModifierFunction(&JsonModifier::mod, modName, &modDesc);

    // Type Modifiers
    bindModifierFunction(&JsonModifier::typeAsString, typeAsStringName, &typeAsStringDesc);
    bindModifierFunction(&JsonModifier::typeAsNumber, typeAsNumberName, &typeAsNumberDesc);

    // Array Modifiers
    bindModifierFunction(&JsonModifier::length, lengthName, &lengthDesc);
}

bool JsonModifier::parse(std::vector<std::string> const& args, JSON* jsonDoc) {
    if (args.empty()) {
        return false;
    }
    for (auto const& modifier : args) {
        std::string const call = std::string(__FUNCTION__) + " " + modifier;
        if (!modifierFuncTree->parseStr(call, jsonDoc)) {
            return false;
        }
    }
    return true;
}

std::string const JsonModifier::valueKey = "v";

//------------------------------------------
// Functions: Arithmetic Modifiers

bool JsonModifier::add(std::span<std::string const> const& args, JSON* jsonDoc) {
    auto numbers = args.subspan(1); // First argument is the modifier name
    for (auto const& numStr : numbers) {
        try {
            double num = std::stod(numStr);
            jsonDoc->set_add(valueKey, num);
        } catch (const std::invalid_argument&) {
            return false;
        } catch (const std::out_of_range&) {
            return false;
        }
    }
    return true;
}

std::string const JsonModifier::addName = "add";
std::string const JsonModifier::addDesc = "Adds a numeric value to the current JSON value. "
    "Usage: |add <number1> <number2> ... -> {number}";

bool JsonModifier::multiply(std::span<std::string const> const& args, JSON* jsonDoc) {
    auto numbers = args.subspan(1); // First argument is the modifier name
    for (auto const& numStr : numbers) {
        try {
            double num = std::stod(numStr);
            jsonDoc->set_multiply(valueKey, num);
        } catch (const std::invalid_argument&) {
            return false;
        } catch (const std::out_of_range&) {
            return false;
        }
    }
    return true;
}

std::string const JsonModifier::multiplyName = "multiply";
std::string const JsonModifier::multiplyDesc = "Multiplies the current JSON value by a numeric value. "
    "Usage: |multiply <number1> <number2> ...";

bool JsonModifier::mod(std::span<std::string const> const& args, JSON* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    try {
        double modValue = std::stod(args[1]);
        double currentValue = jsonDoc->get<double>(valueKey, 0.0);
        if (modValue == 0.0) {
            return false; // Modulo by zero is undefined
        }
        double result = std::fmod(currentValue, modValue);
        jsonDoc->set<double>(valueKey, result);
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
}

std::string const JsonModifier::modName = "mod";
std::string const JsonModifier::modDesc = "Calculates the modulo of the current JSON value by a numeric value. "
    "Usage: |mod <number> -> {number}";

//------------------------------------------
// Type Modifiers

bool JsonModifier::typeAsString(std::span<std::string const> const& args, JSON* jsonDoc) {
    switch (jsonDoc->memberType(valueKey)) {
    case JSON::KeyType::value:
        jsonDoc->set<std::string>(valueKey, "value");
        break;
    case JSON::KeyType::array:
        jsonDoc->set<std::string>(valueKey, "array");
        break;
    case JSON::KeyType::object:
        jsonDoc->set<std::string>(valueKey, "object");
        break;
    case JSON::KeyType::null:
    default:
        jsonDoc->set<std::string>(valueKey, "null");
        break;
    }
    return true;
}

std::string const JsonModifier::typeAsStringName = "typeAsString";
std::string const JsonModifier::typeAsStringDesc = "Converts the current JSON value to a string. "
    "Usage: |typeAsString -> {value,array,object}";

bool JsonModifier::typeAsNumber(std::span<std::string const> const& args, JSON* jsonDoc) {
    jsonDoc->set<int>(valueKey, static_cast<int>(jsonDoc->memberType(valueKey)));
    return true;
}

std::string const JsonModifier::typeAsNumberName = "typeAsNumber";
std::string const JsonModifier::typeAsNumberDesc = "Converts the current JSON value to a number. "
    "Usage: |typeAsNumber -> {number}, where the number reflects the enum value JSON::KeyType.";


//------------------------------------------
// Array Modifiers

bool JsonModifier::length(std::span<std::string const> const& args, JSON* jsonDoc) {
    size_t len = jsonDoc->memberSize(valueKey);
    jsonDoc->set(valueKey, static_cast<double>(len));
    return true;
}

std::string const JsonModifier::lengthName = "length";
std::string const JsonModifier::lengthDesc = "Gets the length of the array in the current JSON value. "
    "Usage: |length -> {number}";

bool JsonModifier::at(std::span<std::string const> const& args, JSON* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    try {
        size_t index = static_cast<size_t>(std::stoul(args[1]));
        size_t arraySize = jsonDoc->memberSize(valueKey);
        if (index >= arraySize) {
            return false; // Index out of bounds
        }
        /**
         * @todo Implement a getVariant method to avoid type issues like this.
         *       simpleValue getVariant(std::string const& key);    // We need to make sure the value actually exists
         *       void setVariant(std::string const& key, simpleValue const& val);
         *       RjDirectAccess::valueType(key); // returns the type of the value stored at key, so we can handle it accordingly
         *       // So we need a large enum for all possible types that rapidjson can differentiate between with IsX() methods.
         */
        auto value = jsonDoc->get<std::string>(valueKey + "[" + std::to_string(index) + "]");
        jsonDoc->set(valueKey, value);
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    }
}
} // namespace Nebulite::Utility