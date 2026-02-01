#include "TransformationModule/Casting.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::TransformationModule {

void Casting::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&Casting::toInt, toIntName, toIntDesc);
    BIND_TRANSFORMATION_STATIC(&Casting::toString, toStringName, toStringDesc);
    BIND_TRANSFORMATION_STATIC(&Casting::toBool, toBoolName, toBoolDesc);
    BIND_TRANSFORMATION_STATIC(&Casting::toDouble, toDoubleName, toDoubleDesc);
    BIND_TRANSFORMATION_STATIC(&Casting::toBoolString, toBoolStringName, toBoolStringDesc);
}

bool Casting::toInt(Core::JsonScope* jsonDoc) {
    // Check if it has any value
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        return false;
    }

    // Get current value as double and cast to int
    auto const currentValue = jsonDoc->get<double>(rootKey, 0.0);
    auto const valueAsInt = static_cast<int>(currentValue);
    jsonDoc->set<int>(rootKey, valueAsInt);
    return true;
}

bool Casting::toString(Core::JsonScope* jsonDoc) {
    // Check if it has any value
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        return false;
    }

    auto const valAsString = jsonDoc->get<std::string>(rootKey, "");
    jsonDoc->set<std::string>(rootKey, valAsString);
    return true;
}

bool Casting::toBool(Core::JsonScope* jsonDoc) {
    // Check if it has any value
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        return false;
    }

    // Try to interpret the current value as string first
    static auto supportedTrueValues = std::set<std::string>{"true", "1", "yes", "on"};
    static auto supportedFalseValues = std::set<std::string>{"false", "0", "no", "off"};

    auto const currentValueStr = jsonDoc->get<std::string>(rootKey, "");
    std::string valueLower;
    std::ranges::transform(currentValueStr, std::back_inserter(valueLower), [](unsigned char const c) {
            return std::tolower(c);
    });
    if (supportedTrueValues.contains(valueLower)) {
        jsonDoc->set<bool>(rootKey, true);
        return true;
    }
    if (supportedFalseValues.contains(valueLower)) {
        jsonDoc->set<bool>(rootKey, false);
        return true;
    }

    // Fallback: get as bool directly
    bool const boolValue = jsonDoc->get<bool>(rootKey, false);
    jsonDoc->set<bool>(rootKey, boolValue);
    return true;
}

bool Casting::toDouble(Core::JsonScope* jsonDoc) {
    // Check if it has any value
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        return false;
    }

    // Get current value as double
    auto const currentValue = jsonDoc->get<double>(rootKey, 0.0);
    jsonDoc->set<double>(rootKey, currentValue);
    return true;
}

bool Casting::toBoolString(Core::JsonScope* jsonDoc) {
    // Check if it has any value
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        return false;
    }

    // Get current value as bool
    bool const boolValue = jsonDoc->get<bool>(rootKey, false);
    jsonDoc->set<std::string>(rootKey, boolValue ? "true" : "false");
    return true;
}

} // namespace Nebulite::TransformationModule
