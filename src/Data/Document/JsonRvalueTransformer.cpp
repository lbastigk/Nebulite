//------------------------------------------
// Includes

#include "Nebulite.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/JsonRvalueTransformer.hpp"

//------------------------------------------
namespace Nebulite::Data {

JsonRvalueTransformer::JsonRvalueTransformer() {
    transformationFuncTree = std::make_unique<Interaction::Execution::FuncTree<bool, JSON*>>("JSON rvalue transformation FuncTree", true, false);

    //------------------------------------------
    // Bind transformation functions

    // Functions: Arithmetic
    bindTransformationFunction(&JsonRvalueTransformer::add, addName, addDesc);
    bindTransformationFunction(&JsonRvalueTransformer::mod, modName, modDesc);
    bindTransformationFunction(&JsonRvalueTransformer::multiply, multiplyName, multiplyDesc);
    bindTransformationFunction(&JsonRvalueTransformer::pow, powName, powDesc);

    // Functions: Array-related
    bindTransformationFunction(&JsonRvalueTransformer::at, atName, atDesc);
    bindTransformationFunction(&JsonRvalueTransformer::length, lengthName, lengthDesc);
    bindTransformationFunction(&JsonRvalueTransformer::reverse, reverseName, reverseDesc);

    // Functions: Casting
    bindTransformationFunction(&JsonRvalueTransformer::toInt, toIntName, toIntDesc);
    bindTransformationFunction(&JsonRvalueTransformer::toString, toStringName, toStringDesc);

    // Functions: Collection
    //bindTransformationFunction(&JsonRvalueTransformer::filter, filterName, &filterDesc);
    bindTransformationFunction(&JsonRvalueTransformer::map, mapName, mapDesc);

    // Functions: Debugging
    bindTransformationFunction(&JsonRvalueTransformer::echo, echoName, echoDesc);
    bindTransformationFunction(&JsonRvalueTransformer::print, printName, printDesc);

    // Functions: Type-related
    bindTransformationFunction(&JsonRvalueTransformer::typeAsNumber, typeAsNumberName, typeAsNumberDesc);
    bindTransformationFunction(&JsonRvalueTransformer::typeAsString, typeAsStringName, typeAsStringDesc);
}

bool JsonRvalueTransformer::parse(std::vector<std::string> const& args, JSON* jsonDoc) {
    static std::string const funcName = __FUNCTION__;
    if (args.empty()) {
        return false;
    }
    return std::ranges::all_of(args, [&](std::string const& transformation) {
        std::string const call = funcName + " " + transformation;
        return transformationFuncTree->parseStr(call, jsonDoc);
    });
}

//------------------------------------------
// Functions: Arithmetic

// NOLINTNEXTLINE
bool JsonRvalueTransformer::add(std::span<std::string const> const& args, JSON* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    auto numbers = args.subspan(1); // First argument is the transformation name
    for (auto const& numStr : numbers) {
        try {
            double num = std::stod(numStr);
            jsonDoc->set_add(valueKey, num);
        } catch (...) {
            return false;
        }
    }
    return true;
}

// NOLINTNEXTLINE
bool JsonRvalueTransformer::multiply(std::span<std::string const> const& args, JSON* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    auto numbers = args.subspan(1); // First argument is the transformation name
    for (auto const& numStr : numbers) {
        try {
            double num = std::stod(numStr);
            jsonDoc->set_multiply(valueKey, num);
        } catch (...) {
            return false;
        }
    }
    return true;
}

// NOLINTNEXTLINE
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
    } catch (...) {
        return false;
    }
}

// NOLINTNEXTLINE
bool JsonRvalueTransformer::pow(std::span<std::string const> const& args, JSON* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        double exponent = std::stod(args[1]);
        auto currentValue = jsonDoc->get<double>(valueKey, 0.0);
        double result = std::pow(currentValue, exponent);
        jsonDoc->set<double>(valueKey, result);
        return true;
    } catch (...) {
        return false;
    }
}

//------------------------------------------
// Functions: Array-related

// NOLINTNEXTLINE
bool JsonRvalueTransformer::at(std::span<std::string const> const& args, JSON* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        auto index = static_cast<size_t>(std::stoul(args[1]));
        size_t arraySize = jsonDoc->memberSize(valueKey);
        if (index >= arraySize) {
            return false; // Index out of bounds
        }
        JSON temp = jsonDoc->getSubDoc(std::string(valueKey) + "[" + std::to_string(index) + "]");
        jsonDoc->setSubDoc(valueKey, temp);
        return true;
    } catch (...) {
        return false;
    }
}

// NOLINTNEXTLINE
bool JsonRvalueTransformer::length(std::span<std::string const> const& args, JSON* jsonDoc) {
    size_t len = jsonDoc->memberSize(valueKey);
    jsonDoc->set(valueKey, static_cast<double>(len));
    return true;
}

// NOLINTNEXTLINE
bool JsonRvalueTransformer::reverse(std::span<std::string const> const& args, JSON* jsonDoc) {
    if (jsonDoc->memberType(valueKey) == JSON::KeyType::value) {
        // Single value, we wrap it into an array
        JSON tmp = jsonDoc->getSubDoc(valueKey);
        std::string const key = std::string(valueKey) + "[0]";
        jsonDoc->setSubDoc(key.c_str(), tmp);
    }
    if (jsonDoc->memberType(valueKey) != JSON::KeyType::array) {
        // Not an array, but we still return true to avoid breaking chains
        return true;
    }
    size_t arraySize = jsonDoc->memberSize(valueKey);
    JSON tmp = jsonDoc->getSubDoc(valueKey);
    for (size_t i = 0; i < arraySize; ++i) {
        std::string const key = std::string(valueKey) + "[" + std::to_string(i) + "]";
        JSON element = tmp.getSubDoc("[" + std::to_string(arraySize - 1 - i) + "]");
        jsonDoc->setSubDoc(key.c_str(), element);
    }
    return true;
}

//------------------------------------------
// Functions: Casting

// NOLINTNEXTLINE
bool JsonRvalueTransformer::toInt(std::span<std::string const> const& args, JSON* jsonDoc) {
    auto currentValue = jsonDoc->get<double>(valueKey, 0.0);
    auto valueAsInt = static_cast<int>(currentValue);
    jsonDoc->set<int>(valueKey, valueAsInt);
    return true;
}

// NOLINTNEXTLINE
bool JsonRvalueTransformer::toString(std::span<std::string const> const& args, JSON* jsonDoc) {
    auto const valAsString = jsonDoc->get<std::string>(valueKey, "");
    jsonDoc->set<std::string>(valueKey, valAsString);
    return true;
}

//------------------------------------------
// Functions: Collection

// NOLINTNEXTLINE
bool JsonRvalueTransformer::map(std::span<std::string const> const& args, JSON* jsonDoc) {
    if (jsonDoc->memberType(valueKey) == JSON::KeyType::value) {
        // Single value, we wrap it into an array
        JSON tmp = jsonDoc->getSubDoc(valueKey);
        std::string const key = std::string(valueKey) + "[0]";
        jsonDoc->setSubDoc(key.c_str(), tmp);
    }
    // Now we expect an array
    if (jsonDoc->memberType(valueKey) != JSON::KeyType::array) {
        return false; // Not an array
    }
    // Re-join args into a single transformation command
    std::string cmd = __FUNCTION__;
    for (auto const& arg : args.subspan(1)) {
        cmd += " ";
        cmd += arg;
    }

    size_t arraySize = jsonDoc->memberSize(valueKey);
    for (uint32_t idx = 0; idx < arraySize; ++idx) {
        // Set temp document with current element
        std::string const elementKey = std::string(valueKey) + "[" + std::to_string(idx) + "]";
        JSON element = jsonDoc->getSubDoc(elementKey);
        JSON tempDoc;
        tempDoc.setSubDoc(valueKey, element);

        // Parse transformation command
        if (!transformationFuncTree->parseStr(cmd, &tempDoc)) {
            tempDoc.removeKey(valueKey);
        }
        JSON transformedElement = tempDoc.getSubDoc(valueKey);
        jsonDoc->setSubDoc(elementKey, transformedElement);
    }
    return true;
}

//------------------------------------------
// Functions: Debugging

// NOLINTNEXTLINE
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

// NOLINTNEXTLINE
bool JsonRvalueTransformer::print(std::span<std::string const> const& args, JSON* jsonDoc) {
    // Print to cout, no modifications
    if (args.size() > 1) {
        Nebulite::Utility::Capture::cout() << jsonDoc->serialize(args[1]) << Nebulite::Utility::Capture::endl;
    } else {
        Nebulite::Utility::Capture::cout() << jsonDoc->serialize() << Nebulite::Utility::Capture::endl;
    }
    return true;
}

//------------------------------------------
// Functions: Type-related

// NOLINTNEXTLINE
bool JsonRvalueTransformer::typeAsNumber(std::span<std::string const> const& args, JSON* jsonDoc) {
    jsonDoc->set<int>(valueKey, static_cast<int>(jsonDoc->memberType(valueKey)));
    return true;
}

// NOLINTNEXTLINE
bool JsonRvalueTransformer::typeAsString(std::span<std::string const> const& args, JSON* jsonDoc) {
    // TODO: Add a getTypeAsString function to JSON class to avoid code duplication
    //       - array -> "array::size"
    //       - object -> "object::size"
    //       - null -> "null"
    //       - string -> "value:string:size"
    //       - int -> "value:int:bitwidth"
    //       - double -> "value:double:bitwidth"
    //       - bool -> "value:bool"
    //       etc...
    //       <baseType>:<subType>:<sizeInfo>
    //       e.g.: "value:int:32" or "value:string:10"
    //       Perhaps with additional arg to control the format?
    switch (jsonDoc->memberType(valueKey)) {
    case JSON::KeyType::value: {
        // General type is "value", but we can be more specific by using getVariant or even better:
        // TODO: see above comment
        jsonDoc->set<std::string>(valueKey, "value");
    }
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

} // namespace Nebulite::Utility
