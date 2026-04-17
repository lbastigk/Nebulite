#include "Data/Document/KeyType.hpp"
#include "Data/Document/JsonScope.hpp"
#include "Module/Transformation/Array.hpp"


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
bool Array::ensureArray(Data::JsonScope* jsonDoc) {
    // Cache the original member type to avoid the analyzer thinking the second branch is unreachable
    if (jsonDoc->memberType(rootKey) == Data::KeyType::array) {
        return true;
    }

    // Single value, wrap into an array
    auto const key = rootKey + "[0]";
    jsonDoc->moveMember(rootKey, key); // Move the original value to the new array index

    // Return whether wrapping succeeded
    auto const newType = jsonDoc->memberType(rootKey);
    return newType == Data::KeyType::array;
}

bool Array::at(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        if (!ensureArray(jsonDoc)) {
            return false;
        }
        auto const index = std::stoul(args[1]);
        if (index >= jsonDoc->memberSize(rootKey)) {
            return false; // Index out of bounds
        }
        Data::JSON const temp = jsonDoc->getSubDoc(rootKey + "[" + std::to_string(index) + "]");
        jsonDoc->setSubDoc(rootKey, temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool Array::length(Data::JsonScope* jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    size_t const len = jsonDoc->memberSize(rootKey);
    jsonDoc->set(rootKey, static_cast<uint64_t>(len));
    return true;
}

bool Array::reverse(Data::JsonScope* jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    size_t const arraySize = jsonDoc->memberSize(rootKey);
    Data::JSON const tmp = jsonDoc->getSubDoc(rootKey);
    for (size_t i = 0; i < arraySize; ++i) {
        auto const key = rootKey + "[" + std::to_string(i) + "]";
        Data::JSON element = tmp.getSubDoc("[" + std::to_string(arraySize - 1 - i) + "]");
        jsonDoc->setSubDoc(key, element);
    }
    return true;
}

bool Array::first(Data::JsonScope* jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    if (jsonDoc->memberSize(rootKey) == 0) {
        return false; // Empty array
    }
    Data::JSON const firstElement = jsonDoc->getSubDoc(rootKey + "[0]");
    jsonDoc->setSubDoc(rootKey, firstElement);
    return true;
}

bool Array::last(Data::JsonScope* jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    size_t const arraySize = jsonDoc->memberSize(rootKey);
    if (arraySize == 0) {
        return false; // Empty array
    }
    Data::JSON const lastElement = jsonDoc->getSubDoc(rootKey + "[" + std::to_string(arraySize - 1) + "]");
    jsonDoc->setSubDoc(rootKey, lastElement);
    return true;
}

bool Array::push(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    size_t const arraySize = jsonDoc->memberSize(rootKey);
    auto const key = rootKey + "[" + std::to_string(arraySize) + "]";
    jsonDoc->set(key, Utility::StringHandler::recombineArgs(args.subspan(1)));
    return true;
}

bool Array::pushNumber(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        double const number = std::stod(args[1]);
        if (!ensureArray(jsonDoc)) {
            return false;
        }
        size_t const arraySize = jsonDoc->memberSize(rootKey);
        auto const key = rootKey + "[" + std::to_string(arraySize) + "]";
        jsonDoc->set(key, number);
        return true;
    } catch (...) {
        return false;
    }
}

bool Array::subspan(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() > 3) {
        return false;
    }
    // Move the original array to a temp key, so we can modify the original key to be the subspan without losing data
    size_t const originalSize = jsonDoc->memberSize(rootKey);
    auto const tmpKey = rootKey + "[" + std::to_string(originalSize) + "]";
    jsonDoc->copyMember(rootKey, tmpKey); // Using copy, as current moveMember is more of a copy+delete and thus slower

    // Setup start index and length, with length defaulting to the rest of the array if not provided
    size_t const startIndex = std::stoul(args[1]);
    size_t const length = args.size() > 2 ? std::stoul(args[2]) : originalSize; // Set high enough length if not provided

    // If start index is larger than original size, return empty array
    if (startIndex >= originalSize) {
        jsonDoc->setEmptyArray(rootKey);
        return true;
    }

    // Starting at startIndex, until length, as long as it's smaller than originalSize
    size_t index = 0;
    std::ranges::for_each(
        std::views::iota(startIndex, std::min(startIndex + length, originalSize)),
        [&](size_t const& i) {
            auto const key = rootKey + "[" + std::to_string(index++) + "]";
            jsonDoc->copyMember(tmpKey + "[" + std::to_string(i) + "]", key);
        }
    );

    // Remove any remaining elements from the original array that are not in the subspan + the allocated temp key
    for (size_t i = originalSize; i >= index; i--) {
        jsonDoc->removeMember(rootKey + "[" + std::to_string(i) + "]");
    }
    return true;
}

} // namespace Nebulite::TransformationModule
