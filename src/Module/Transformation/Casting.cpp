#include "Nebulite.hpp"
#include "Module/Transformation/Casting.hpp"


namespace Nebulite::Module::Transformation {

void Casting::bindTransformations() {
    bindTransformation(&Casting::toInt, toIntName, toIntDesc);
    bindTransformation(&Casting::toString, toStringName, toStringDesc);
    bindTransformation(&Casting::toBool, toBoolName, toBoolDesc);
    bindTransformation(&Casting::toDouble, toDoubleName, toDoubleDesc);
    bindTransformation(&Casting::toBoolString, toBoolStringName, toBoolStringDesc);
    bindTransformation(&Casting::roundUp, roundUpName, roundUpDesc);
    bindTransformation(&Casting::roundDown, roundDownName, roundDownDesc);
    bindTransformation(&Casting::round, roundName, roundDesc);
}

bool Casting::toInt(Data::JsonScope* jsonDoc) {
    // Check if it has any value
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        return false;
    }

    // Get current value as double and cast to int
    auto const currentValue = jsonDoc->get<double>(rootKey).value_or(0.0);
    auto const valueAsInt = static_cast<int>(currentValue);
    jsonDoc->set<int>(rootKey, valueAsInt);
    return true;
}

bool Casting::toString(Data::JsonScope* jsonDoc) {
    // Check if it has any value
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        return false;
    }

    auto const valAsString = jsonDoc->get<std::string>(rootKey).value_or("");
    jsonDoc->set<std::string>(rootKey, valAsString);
    return true;
}

bool Casting::toBool(Data::JsonScope* jsonDoc) {
    // Check if it has any value
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        return false;
    }

    // Try to interpret the current value as string first
    static auto supportedTrueValues = std::set<std::string>{"true", "1", "yes", "on"};
    static auto supportedFalseValues = std::set<std::string>{"false", "0", "no", "off"};

    auto const currentValueStr = jsonDoc->get<std::string>(rootKey).value_or("");
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
    bool const boolValue = jsonDoc->get<bool>(rootKey).value_or(false);
    jsonDoc->set<bool>(rootKey, boolValue);
    return true;
}

bool Casting::toDouble(Data::JsonScope* jsonDoc) {
    // Check if it has any value
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        return false;
    }

    // Get current value as double
    auto const currentValue = jsonDoc->get<double>(rootKey).value_or(0.0);
    jsonDoc->set<double>(rootKey, currentValue);
    return true;
}

bool Casting::toBoolString(Data::JsonScope* jsonDoc) {
    // Check if it has any value
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        return false;
    }

    // Get current value as bool
    bool const boolValue = jsonDoc->get<bool>(rootKey).value_or(false);
    jsonDoc->set<std::string>(rootKey, boolValue ? "true" : "false");
    return true;
}

//------------------------------------------

bool Casting::roundUp(Data::JsonScope* jsonDoc) {
    // Check if it has any value
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        return false;
    }

    // Get current value as double and round up
    auto const currentValue = jsonDoc->get<double>(rootKey).value_or(0.0);
    auto const roundedValue = static_cast<int>(std::ceil(currentValue));
    jsonDoc->set<int>(rootKey, roundedValue);
    return true;
}

bool Casting::roundDown(Data::JsonScope* jsonDoc) {
    // Check if it has any value
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        return false;
    }

    // Get current value as double and round down
    auto const currentValue = jsonDoc->get<double>(rootKey).value_or(0.0);
    auto const roundedValue = static_cast<int>(std::floor(currentValue));
    jsonDoc->set<int>(rootKey, roundedValue);
    return true;
}

bool Casting::round(Data::JsonScope* jsonDoc) {
    // Check if it has any value
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        return false;
    }

    // Get current value as double and round
    auto const currentValue = jsonDoc->get<double>(rootKey).value_or(0.0);
    auto const roundedValue = static_cast<int>(std::round(currentValue));
    jsonDoc->set<int>(rootKey, roundedValue);
    return true;
}

} // namespace Nebulite::Module::Transformation
