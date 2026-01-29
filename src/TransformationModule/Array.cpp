#include "TransformationModule/Array.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::TransformationModule {

void Array::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&Array::ensureArray, ensureArrayName, ensureArrayDesc);
    BIND_TRANSFORMATION_STATIC(&Array::at, atName, atDesc);
    BIND_TRANSFORMATION_STATIC(&Array::length, lengthName, lengthDesc);
    BIND_TRANSFORMATION_STATIC(&Array::reverse, reverseName, reverseDesc);
    BIND_TRANSFORMATION_STATIC(&Array::first, firstName, firstDesc);
    BIND_TRANSFORMATION_STATIC(&Array::last, lastName, lastDesc);
    BIND_TRANSFORMATION_STATIC(&Array::push, pushName, pushDesc);
    BIND_TRANSFORMATION_STATIC(&Array::pushNumber, pushNumberName, pushNumberDesc);
    BIND_TRANSFORMATION_STATIC(&Array::subspan, subspanName, subspanDesc);
}

// Clang marks this function as having an unreachable branch,
// because it thinks the first branch always returns true?
// Disable the warning for this function.
// NOLINTNEXTLINE
bool Array::ensureArray(Core::JsonScope* jsonDoc) {
    // Cache the original member type to avoid the analyzer thinking the second branch is unreachable
    if (jsonDoc->memberType(valueKey) == Data::KeyType::array) {
        return true;
    }

    // Single value, wrap into an array
    Data::JSON const tmp = jsonDoc->getSubDoc(valueKey);
    auto const key = valueKey + "[0]";
    jsonDoc->setSubDoc(key, tmp);

    // Return whether wrapping succeeded
    return jsonDoc->memberType(valueKey) == Data::KeyType::array;
}

bool Array::at(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        if (!ensureArray(jsonDoc)) {
            return false;
        }
        auto const index = std::stoul(args[1]);
        if (index >= jsonDoc->memberSize(valueKey)) {
            return false; // Index out of bounds
        }
        Data::JSON const temp = jsonDoc->getSubDoc(valueKey + "[" + std::to_string(index) + "]");
        jsonDoc->setSubDoc(valueKey, temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool Array::length(Core::JsonScope* jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    size_t const len = jsonDoc->memberSize(valueKey);
    jsonDoc->set(valueKey, static_cast<double>(len));
    return true;
}

bool Array::reverse(Core::JsonScope* jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    size_t const arraySize = jsonDoc->memberSize(valueKey);
    Data::JSON const tmp = jsonDoc->getSubDoc(valueKey);
    for (size_t i = 0; i < arraySize; ++i) {
        auto const key = valueKey + "[" + std::to_string(i) + "]";
        Data::JSON element = tmp.getSubDoc("[" + std::to_string(arraySize - 1 - i) + "]");
        jsonDoc->setSubDoc(key, element);
    }
    return true;
}

bool Array::first(Core::JsonScope* jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    if (jsonDoc->memberSize(valueKey) == 0) {
        return false; // Empty array
    }
    Data::JSON const firstElement = jsonDoc->getSubDoc(valueKey + "[0]");
    jsonDoc->setSubDoc(valueKey, firstElement);
    return true;
}

bool Array::last(Core::JsonScope* jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    size_t const arraySize = jsonDoc->memberSize(valueKey);
    if (arraySize == 0) {
        return false; // Empty array
    }
    Data::JSON const lastElement = jsonDoc->getSubDoc(valueKey + "[" + std::to_string(arraySize - 1) + "]");
    jsonDoc->setSubDoc(valueKey, lastElement);
    return true;
}

bool Array::push(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    size_t const arraySize = jsonDoc->memberSize(valueKey);
    auto const key = valueKey + "[" + std::to_string(arraySize) + "]";
    jsonDoc->set(key, Utility::StringHandler::recombineArgs(args.subspan(1)));
    return true;
}

bool Array::pushNumber(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        double const number = std::stod(args[1]);
        if (!ensureArray(jsonDoc)) {
            return false;
        }
        size_t const arraySize = jsonDoc->memberSize(valueKey);
        auto const key = valueKey + "[" + std::to_string(arraySize) + "]";
        jsonDoc->set(key, number);
        return true;
    } catch (...) {
        return false;
    }
}

bool Array::subspan(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() > 3) {
        return false;
    }
    size_t const originalSize = jsonDoc->memberSize(valueKey);
    Data::JSON const copy = jsonDoc->getSubDoc(valueKey);

    size_t const startIndex = std::stoul(args[1]);
    size_t const length = args.size() > 2 ? std::stoul(args[2]) : originalSize; // Set high enough length if not provided

    // Clear current array
    jsonDoc->setEmptyArray(valueKey);

    // If start index is larger than original size, return empty array
    if (startIndex >= originalSize) {
        return true;
    }

    // Starting at startIndex, until length, as long as its smaller than originalSize
    std::ranges::for_each(
        std::views::iota(startIndex, std::min(startIndex + length, originalSize)),
        [&](size_t const& i) {
            auto const key = valueKey + "[" + std::to_string(jsonDoc->memberSize(valueKey)) + "]";
            Data::JSON const element = copy.getSubDoc("[" + std::to_string(i) + "]");
            jsonDoc->setSubDoc(key, element);
        }
    );
    return true;
}

} // namespace Nebulite::TransformationModule
