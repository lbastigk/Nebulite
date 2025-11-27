//------------------------------------------
// Includes

#include "Nebulite.hpp"
#include "Utility/JSON.hpp"
#include "Utility/JsonRvalueTransformer.hpp"

//------------------------------------------
namespace Nebulite::Utility {

JsonRvalueTransformer::JsonRvalueTransformer() {
    transformationFuncTree = std::make_unique<Interaction::Execution::FuncTree<bool, JSON*>>("JSON rvalue transformation FuncTree", true, false);

    //------------------------------------------
    // Bind transformation functions

    // Functions: Arithmetic
    bindTransformationFunction(&JsonRvalueTransformer::add, addName, &addDesc);
    bindTransformationFunction(&JsonRvalueTransformer::multiply, multiplyName, &multiplyDesc);
    bindTransformationFunction(&JsonRvalueTransformer::mod, modName, &modDesc);

    // Functions: Array-related
    bindTransformationFunction(&JsonRvalueTransformer::at, atName, &atDesc);
    bindTransformationFunction(&JsonRvalueTransformer::length, lengthName, &lengthDesc);

    // Functions: Casting
    bindTransformationFunction(&JsonRvalueTransformer::toInt, toIntName, &toIntDesc);

    // Functions: Debugging
    bindTransformationFunction(&JsonRvalueTransformer::echo, echoName, &echoDesc);
    bindTransformationFunction(&JsonRvalueTransformer::print, printName, &printDesc);

    // Functions: Type-related
    bindTransformationFunction(&JsonRvalueTransformer::typeAsString, typeAsStringName, &typeAsStringDesc);
    bindTransformationFunction(&JsonRvalueTransformer::typeAsNumber, typeAsNumberName, &typeAsNumberDesc);
}

bool JsonRvalueTransformer::parse(std::vector<std::string> const& args, JSON* jsonDoc) {
    static std::string const funcName = __FUNCTION__;
    if (args.empty()) {
        return false;
    }
    if (!std::ranges::all_of(args, [&](std::string const& transformation) {
        std::string const call = funcName + " " + transformation;
        return transformationFuncTree->parseStr(call, jsonDoc);
    })) {
        return false;
    }
    return true;
}

// TODO: test if using no key "" works as intended in all cases
std::string const JsonRvalueTransformer::valueKey = "";

//------------------------------------------
// Functions: Arithmetic

bool JsonRvalueTransformer::add(std::span<std::string const> const& args, JSON* jsonDoc) {
    auto numbers = args.subspan(1); // First argument is the transformation name
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

std::string const JsonRvalueTransformer::addName = "add";
std::string const JsonRvalueTransformer::addDesc = "Adds a numeric value to the current JSON value. "
    "Usage: |add <number1> <number2> ... -> {number}";

bool JsonRvalueTransformer::multiply(std::span<std::string const> const& args, JSON* jsonDoc) {
    auto numbers = args.subspan(1); // First argument is the transformation name
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

std::string const JsonRvalueTransformer::multiplyName = "multiply";
std::string const JsonRvalueTransformer::multiplyDesc = "Multiplies the current JSON value by a numeric value. "
    "Usage: |multiply <number1> <number2> ...";

bool JsonRvalueTransformer::mod(std::span<std::string const> const& args, JSON* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    try {
        double modValue = std::stod(args[1]);
        auto currentValue = jsonDoc->get<double>(valueKey, 0.0);
        if (std::fabs(modValue) < std::numeric_limits<double>::epsilon()) {
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

std::string const JsonRvalueTransformer::modName = "mod";
std::string const JsonRvalueTransformer::modDesc = "Calculates the modulo of the current JSON value by a numeric value. "
    "Usage: |mod <number> -> {number}";

bool JsonRvalueTransformer::pow(std::span<std::string const> const& args, JSON* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    try {
        double exponent = std::stod(args[1]);
        auto currentValue = jsonDoc->get<double>(valueKey, 0.0);
        double result = std::pow(currentValue, exponent);
        jsonDoc->set<double>(valueKey, result);
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
}

std::string const JsonRvalueTransformer::powName = "pow";
std::string const JsonRvalueTransformer::powDesc = "Raises the current JSON value to the power of a numeric value. "
    "Usage: |pow <exponent> -> {number}";

//------------------------------------------
// Functions: Array-related

bool JsonRvalueTransformer::length(std::span<std::string const> const& args, JSON* jsonDoc) {
    size_t len = jsonDoc->memberSize(valueKey);
    jsonDoc->set(valueKey, static_cast<double>(len));
    return true;
}

std::string const JsonRvalueTransformer::lengthName = "length";
std::string const JsonRvalueTransformer::lengthDesc = "Gets the length of the array in the current JSON value. "
    "Usage: |length -> {number}";

bool JsonRvalueTransformer::at(std::span<std::string const> const& args, JSON* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    try {
        auto index = static_cast<size_t>(std::stoul(args[1]));
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

std::string const JsonRvalueTransformer::atName = "at";
std::string const JsonRvalueTransformer::atDesc = "Gets the element at the specified index from the array in the current JSON value. "
    "Usage: |at <index> -> {value}";

//------------------------------------------
// Functions: Casting

bool JsonRvalueTransformer::toInt(std::span<std::string const> const& args, JSON* jsonDoc) {
    try {
        auto currentValue = jsonDoc->get<double>(valueKey, 0.0);
        int intValue = static_cast<int>(currentValue);
        jsonDoc->set<int>(valueKey, intValue);
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
}

std::string const JsonRvalueTransformer::toIntName = "toInt";
std::string const JsonRvalueTransformer::toIntDesc = "Converts the current JSON value to an integer. "
    "Usage: |toInt -> {number}";

bool JsonRvalueTransformer::toString(std::span<std::string const> const& args, JSON* jsonDoc) {
    std::string const val = jsonDoc->get<std::string>(valueKey, "");
    jsonDoc->set<std::string>(valueKey, val);
    return true;
}

std::string const JsonRvalueTransformer::toStringName = "toString";
std::string const JsonRvalueTransformer::toStringDesc = "Converts the current JSON value to a string. "
    "doesn't fail, defaults to empty string. "
    "Usage: |toString -> {string}";

//------------------------------------------
// Functions: Debugging

bool JsonRvalueTransformer::echo(std::span<std::string const> const& args, JSON* jsonDoc) {
    // Echo args to cout
    for (size_t i = 1; i < args.size(); ++i) {
        Nebulite::Utility::Capture::cout() << args[i];
        if (i < args.size() - 1) {
            Nebulite::Utility::Capture::cout() << " ";
        }
    }
    Nebulite::Utility::Capture::cout() << Nebulite::Utility::Capture::endl;
    return true;
}

std::string const JsonRvalueTransformer::echoName = "echo";
std::string const JsonRvalueTransformer::echoDesc = "Echoes the provided arguments to the console, with newline. "
    "Usage: |echo <arg1> <arg2> ...";

bool JsonRvalueTransformer::print(std::span<std::string const> const& args, JSON* jsonDoc) {
    // Print to cout, no modifications
    if (args.size() > 1) {
        std::string key = args[1];
        Nebulite::Utility::Capture::cout() << jsonDoc->serialize(key) << Nebulite::Utility::Capture::endl;
    }
    else {
        Nebulite::Utility::Capture::cout() << jsonDoc->serialize() << Nebulite::Utility::Capture::endl;
    }
    return true;
}

std::string const JsonRvalueTransformer::printName = "print";
std::string const JsonRvalueTransformer::printDesc = "Prints the current JSON value to the console. "
    "Usage: |print";

//------------------------------------------
// Functions: Type-related

bool JsonRvalueTransformer::typeAsString(std::span<std::string const> const& args, JSON* jsonDoc) {
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

std::string const JsonRvalueTransformer::typeAsStringName = "typeAsString";
std::string const JsonRvalueTransformer::typeAsStringDesc = "Converts the current JSON value to a string. "
    "Usage: |typeAsString -> {value,array,object}";

bool JsonRvalueTransformer::typeAsNumber(std::span<std::string const> const& args, JSON* jsonDoc) {
    jsonDoc->set<int>(valueKey, static_cast<int>(jsonDoc->memberType(valueKey)));
    return true;
}

std::string const JsonRvalueTransformer::typeAsNumberName = "typeAsNumber";
std::string const JsonRvalueTransformer::typeAsNumberDesc = "Converts the current JSON value to a number. "
    "Usage: |typeAsNumber -> {number}, where the number reflects the enum value JSON::KeyType.";


} // namespace Nebulite::Utility