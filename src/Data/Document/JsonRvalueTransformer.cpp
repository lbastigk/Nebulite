//------------------------------------------
// Includes

#include "Nebulite.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/JsonRvalueTransformer.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
// Binding helper macro

#define BIND_TRANSFORMATION(foo, name, desc) \
    static_assert(endsWithNewline(desc), "Description must end with a newline character"); \
    Interaction::Execution::DomainModuleBase::bindFunctionStatic(transformationFuncTree.get(), this, foo, name, desc)

//------------------------------------------
namespace Nebulite::Data {

JsonRvalueTransformer::JsonRvalueTransformer() {
    transformationFuncTree = std::make_unique<Interaction::Execution::FuncTree<bool, JsonScope*>>("JSON rvalue transformation FuncTree", true, false);

    //------------------------------------------
    // Bind transformation functions

    // Functions: Arithmetic
    BIND_TRANSFORMATION(&JsonRvalueTransformer::add, addName, addDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::mod, modName, modDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::multiply, multiplyName, multiplyDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::pow, powName, powDesc);

    // Functions: Array-related
    BIND_TRANSFORMATION(&JsonRvalueTransformer::ensureArray, ensureArrayName, ensureArrayDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::at, atName, atDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::length, lengthName, lengthDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::reverse, reverseName, reverseDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::first, firstName, firstDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::last, lastName, lastDesc);

    // Functions: Assertions
    BIND_TRANSFORMATION(&JsonRvalueTransformer::assertNonEmpty, assertNonEmptyName, assertNonEmptyDesc);

    // Functions: Casting
    BIND_TRANSFORMATION(&JsonRvalueTransformer::toInt, toIntName, toIntDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::toString, toStringName, toStringDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::toBool, toBoolName, toBoolDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::toDouble, toDoubleName, toDoubleDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::toBoolString, toBoolStringName, toBoolStringDesc);

    // Functions: Collection
    // BIND_TRANSFORMATION(&JsonRvalueTransformer::filter, filterName, &filterDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::map, mapName, mapDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::get, getName, getDesc);

    // Functions: Debugging
    BIND_TRANSFORMATION(&JsonRvalueTransformer::echo, echoName, echoDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::print, printName, printDesc);

    // Functions: Domain
    BIND_TRANSFORMATION(&JsonRvalueTransformer::nebs, nebsName, nebsDesc);

    // Functions: Statistics
    // ...

    // Functions: String
    // ...

    // Functions: Type-related
    BIND_TRANSFORMATION(&JsonRvalueTransformer::typeAsNumber, typeAsNumberName, typeAsNumberDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::typeAsString, typeAsStringName, typeAsStringDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::serialize, serializeName, serializeDesc);
    BIND_TRANSFORMATION(&JsonRvalueTransformer::deserialize, deserializeName, deserializeDesc);
}

bool JsonRvalueTransformer::parse(std::vector<std::string> const& args, JsonScope* jsonDoc) {
    static std::string const funcName = __FUNCTION__;
    if (args.empty()) {
        return false;
    }
    return std::ranges::all_of(args, [&](std::string const& transformation) {
        std::string const call = funcName + " " + transformation;
        return transformationFuncTree->parseStr(call, jsonDoc);
    });
}

bool JsonRvalueTransformer::parse(std::vector<std::string> const& args, JSON* jsonDoc) {
    auto scope = jsonDoc->shareScope();
    static std::string const funcName = __FUNCTION__;
    if (args.empty()) {
        return false;
    }
    return std::ranges::all_of(args, [&](std::string const& transformation) {
        std::string const call = funcName + " " + transformation;
        return transformationFuncTree->parseStr(call, &scope);
    });
}

//------------------------------------------
// Functions: Arithmetic

bool JsonRvalueTransformer::add(std::span<std::string const> const& args, JsonScope* jsonDoc) {
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

bool JsonRvalueTransformer::multiply(std::span<std::string const> const& args, JsonScope* jsonDoc) {
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

bool JsonRvalueTransformer::mod(std::span<std::string const> const& args, JsonScope* jsonDoc) {
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

bool JsonRvalueTransformer::pow(std::span<std::string const> const& args, JsonScope* jsonDoc) {
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

bool JsonRvalueTransformer::ensureArray(JsonScope* jsonDoc) const {
    // Cache the original member type to avoid the analyzer thinking the second branch is unreachable
    if (auto const originalType = jsonDoc->memberType(valueKey); originalType == KeyType::array) {
        return true;
    }

    // Single value, wrap into an array
    JSON tmp = jsonDoc->getSubDoc(valueKey);
    std::string const key = std::string(valueKey) + "[0]";
    jsonDoc->setSubDoc(key.c_str(), tmp);

    // Return whether wrapping succeeded
    return (jsonDoc->memberType(valueKey) == KeyType::array);
}

bool JsonRvalueTransformer::at(std::span<std::string const> const& args, JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        if (!ensureArray(jsonDoc)) {
            return false;
        }
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

bool JsonRvalueTransformer::length(JsonScope* jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    size_t len = jsonDoc->memberSize(valueKey);
    jsonDoc->set(valueKey, static_cast<double>(len));
    return true;
}

bool JsonRvalueTransformer::reverse(JsonScope* jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
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

bool JsonRvalueTransformer::first(JsonScope* jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    size_t arraySize = jsonDoc->memberSize(valueKey);
    if (arraySize == 0) {
        return false; // Empty array
    }
    JSON firstElement = jsonDoc->getSubDoc(std::string(valueKey) + "[0]");
    jsonDoc->setSubDoc(valueKey, firstElement);
    return true;
}

bool JsonRvalueTransformer::last(JsonScope* jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    size_t arraySize = jsonDoc->memberSize(valueKey);
    if (arraySize == 0) {
        return false; // Empty array
    }
    JSON lastElement = jsonDoc->getSubDoc(std::string(valueKey) + "[" + std::to_string(arraySize - 1) + "]");
    jsonDoc->setSubDoc(valueKey, lastElement);
    return true;
}

//------------------------------------------
// Functions: Assertions

bool JsonRvalueTransformer::assertNonEmpty(JsonScope* jsonDoc) {
    static std::string errorMessage = std::string(__FUNCTION__) + " JSON value is null";

    if (jsonDoc->memberType(valueKey) == KeyType::null) {
        throw std::runtime_error(errorMessage);
        //return false;
    }
    return true;
}

//------------------------------------------
// Functions: Casting

bool JsonRvalueTransformer::toInt(JsonScope* jsonDoc) {
    auto currentValue = jsonDoc->get<double>(valueKey, 0.0);
    auto valueAsInt = static_cast<int>(currentValue);
    jsonDoc->set<int>(valueKey, valueAsInt);
    return true;
}

bool JsonRvalueTransformer::toString(JsonScope* jsonDoc) {
    auto const valAsString = jsonDoc->get<std::string>(valueKey, "");
    jsonDoc->set<std::string>(valueKey, valAsString);
    return true;
}

bool JsonRvalueTransformer::toBool(JsonScope* jsonDoc) {
    // Try to interpret the current value as string first
    static auto supportedTrueValues = std::set<std::string>{"true", "1", "yes", "on"};
    static auto supportedFalseValues = std::set<std::string>{"false", "0", "no", "off"};

    auto const currentValueStr = jsonDoc->get<std::string>(valueKey, "");
    std::string valueLower;
    std::ranges::transform(currentValueStr, std::back_inserter(valueLower), [](unsigned char c) {
            return std::tolower(c);
    });
    if (supportedTrueValues.contains(valueLower)) {
            jsonDoc->set<bool>(valueKey, true);
            return true;
    }
    if (supportedFalseValues.find(valueLower) != supportedFalseValues.end()) {
            jsonDoc->set<bool>(valueKey, false);
            return true;
    }

    // Fallback: get as bool directly
    bool boolValue = jsonDoc->get<bool>(valueKey, false);
    jsonDoc->set<bool>(valueKey, boolValue);
    return true;
}

bool JsonRvalueTransformer::toDouble(JsonScope* jsonDoc) {
    auto currentValue = jsonDoc->get<double>(valueKey, 0.0);
    jsonDoc->set<double>(valueKey, currentValue);
    return true;
}

bool JsonRvalueTransformer::toBoolString(JsonScope* jsonDoc) {
    bool boolValue = jsonDoc->get<bool>(valueKey, false);
    jsonDoc->set<std::string>(valueKey, boolValue ? "true" : "false");
    return true;
}

//------------------------------------------
// Functions: Collection

bool JsonRvalueTransformer::map(std::span<std::string const> const& args, JsonScope* jsonDoc) {
    if (jsonDoc->memberType(valueKey) == KeyType::value) {
        // Single value, we wrap it into an array
        JSON tmp = jsonDoc->getSubDoc(valueKey);
        std::string const key = std::string(valueKey) + "[0]";
        jsonDoc->setSubDoc(key.c_str(), tmp);
    }
    // Now we expect an array
    if (jsonDoc->memberType(valueKey) != KeyType::array) {
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
        JsonScope tempDoc;
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

bool JsonRvalueTransformer::get(std::span<std::string const> const& args, JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    std::string const& key = args[1];
    JSON subDoc = jsonDoc->getSubDoc(key.c_str());
    jsonDoc->setSubDoc(valueKey, subDoc);
    return true;
}

bool JsonRvalueTransformer::getMultiple(std::span<std::string const> const& args, JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    std::vector<std::unique_ptr<JSON>> values;
    for (auto const& key : args.subspan(1)) {
        values.push_back(std::make_unique<JSON>());
        auto subDoc = jsonDoc->getSubDoc(key.c_str());
        values.back()->copyFrom(subDoc);
    }

    // Create result array
    for (size_t i = 0; i < values.size(); ++i) {
        std::string const arrayKey = std::string(valueKey) + "[" + std::to_string(i) + "]";
        jsonDoc->setSubDoc(arrayKey.c_str(), *values[i]);
    }
    return true;
}

//------------------------------------------
// Functions: Debugging

bool JsonRvalueTransformer::echo(std::span<std::string const> const& args) {
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

bool JsonRvalueTransformer::print(std::span<std::string const> const& args, JsonScope* jsonDoc) {
    // Print to cout, no modifications
    if (args.size() > 1) {
        Nebulite::Utility::Capture::cout() << jsonDoc->serialize(args[1]) << Nebulite::Utility::Capture::endl;
    } else {
        Nebulite::Utility::Capture::cout() << jsonDoc->serialize() << Nebulite::Utility::Capture::endl;
    }
    return true;
}

//------------------------------------------
// Functions: Domain

bool JsonRvalueTransformer::nebs(std::span<std::string const> const& args, JsonScope* jsonDoc) {
    std::string cmd = Utility::StringHandler::recombineArgs(args);
    if (jsonDoc->parseStr(cmd) != Constants::ErrorTable::NONE()) {
        return false;
    }
    return true;
}

//------------------------------------------
// Functions: Type-related

bool JsonRvalueTransformer::typeAsNumber(JsonScope* jsonDoc) {
    jsonDoc->set<int>(valueKey, static_cast<int>(jsonDoc->memberType(valueKey)));
    return true;
}

bool JsonRvalueTransformer::typeAsString(JsonScope* jsonDoc) {
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
    case KeyType::value: {
        // General type is "value", but we can be more specific by using getVariant or even better:
        // TODO: see above comment
        jsonDoc->set<std::string>(valueKey, "value");
    }
    break;
    case KeyType::array:
        jsonDoc->set<std::string>(valueKey, "array");
        break;
    case KeyType::object:
        jsonDoc->set<std::string>(valueKey, "object");
        break;
    case KeyType::null:
    default:
        jsonDoc->set<std::string>(valueKey, "null");
        break;
    }
    return true;
}

bool JsonRvalueTransformer::serialize(JsonScope* jsonDoc) {
    std::string const serialized = jsonDoc->serialize();
    jsonDoc->set<std::string>(valueKey, serialized);
    return true;
}

bool JsonRvalueTransformer::deserialize(JsonScope* jsonDoc) {
    std::string const serialized = jsonDoc->get<std::string>(valueKey, "");
    JSON tempDoc;
    if (!JSON::isJsonOrJsonc(serialized)) {
        return false;
    }
    tempDoc.deserialize(serialized);
    jsonDoc->setSubDoc(valueKey, tempDoc);
    return true;
}

} // namespace Nebulite::Data
