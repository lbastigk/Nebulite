//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <cstdint> // NOLINT
#include <ranges>
#include <span>
#include <string>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Module/Transformation/Array.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Array::bindTransformations() {
    // Ranges
    bindTransformation(&Array::at, atName, atDesc);
    bindTransformation(&Array::length, lengthName, lengthDesc);
    bindTransformation(&Array::first, firstName, firstDesc);
    bindTransformation(&Array::last, lastName, lastDesc);
    bindTransformation(&Array::subspan, subspanName, subspanDesc);

    // Modify
    bindTransformation(&Array::reverse, reverseName, reverseDesc);
    bindTransformation(&Array::ensureArray, ensureArrayName, ensureArrayDesc);
    bindTransformation(&Array::push, pushName, pushDesc);
    bindTransformation(&Array::pushNumber, pushNumberName, pushNumberDesc);
    bindTransformation(&Array::enumerate, enumerateName, enumerateDesc);
    bindTransformation(&Array::iota, iotaName, iotaDesc);
}

// Ranges

bool Array::at(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        if (!ensureArray(jsonDoc)) {
            return false;
        }
        auto const index = std::stoul(std::string(args.at(1)));
        if (index >= jsonDoc.memberSize(rootKey)) {
            return false; // Index out of bounds
        }
        Data::JSON const temp = jsonDoc.getSubDoc(rootKey.addIndex(index));
        jsonDoc.setSubDoc(rootKey, temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool Array::length(Data::JsonScope& jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    auto const len = jsonDoc.memberSize(rootKey);
    jsonDoc.set(rootKey, static_cast<uint64_t>(len));
    return true;
}


bool Array::first(Data::JsonScope& jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    if (jsonDoc.memberSize(rootKey) == 0) {
        return false; // Empty array
    }
    Data::JSON const firstElement = jsonDoc.getSubDoc(rootKey.addIndex(0));
    jsonDoc.setSubDoc(rootKey, firstElement);
    return true;
}

bool Array::last(Data::JsonScope& jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    auto const arraySize = jsonDoc.memberSize(rootKey);
    if (arraySize == 0) {
        return false; // Empty array
    }
    Data::JSON const lastElement = jsonDoc.getSubDoc(rootKey.addIndex(arraySize - 1));
    jsonDoc.setSubDoc(rootKey, lastElement);
    return true;
}


bool Array::subspan(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc) {
    if (args.size() > 3) {
        return false;
    }
    // Move the original array to a temp key, so we can modify the original key to be the subspan without losing data
    auto const originalSize = jsonDoc.memberSize(rootKey);
    auto const tmpKey = rootKey.addIndex(originalSize);
    jsonDoc.copyMember(rootKey, tmpKey); // Using copy, as current moveMember is more of a copy+delete and thus slower

    // Setup start index and length, with length defaulting to the rest of the array if not provided
    auto const startIndex = std::stoul(std::string(args.at(1)));
    auto const length = args.size() > 2 ? std::stoul(std::string(args.at(2))) : originalSize; // Set high enough length if not provided

    // If start index is larger than original size, return empty array
    if (startIndex >= originalSize) {
        jsonDoc.setEmptyArray(rootKey);
        return true;
    }

    // Starting at startIndex, until length, as long as it's smaller than originalSize
    std::size_t index = 0;
    std::ranges::for_each(
        std::views::iota(startIndex, std::min(startIndex + length, originalSize)),
        [&](std::size_t const i) {
            auto const key = rootKey.addIndex(index++);
            jsonDoc.copyMember(tmpKey.addIndex(i), key);
        }
    );

    // Remove any remaining elements from the original array that are not in the subspan + the allocated temp key
    for (std::size_t i = originalSize; i >= index; i--) {
        jsonDoc.removeMember(rootKey.addIndex(i));
    }
    return true;
}

// Modify

bool Array::reverse(Data::JsonScope& jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    auto const arraySize = jsonDoc.memberSize(rootKey);
    Data::JSON const tmp = jsonDoc.getSubDoc(rootKey);
    for (std::size_t i = 0; i < arraySize; ++i) {
        auto const key = rootKey.addIndex(i);
        Data::JSON const element = tmp.getSubDoc("[" + std::to_string(arraySize - 1 - i) + "]");
        jsonDoc.setSubDoc(key, element);
    }
    return true;
}

// Clang marks this function as having an unreachable branch,
// because it thinks the first branch always returns true?
// Disable the warning for this function.
// NOLINTNEXTLINE
bool Array::ensureArray(Data::JsonScope& jsonDoc) {
    // Cache the original member type to avoid the analyzer thinking the second branch is unreachable
    if (jsonDoc.memberType(rootKey) == Data::KeyType::array) {
        return true;
    }

    // Single value, wrap into an array
    auto const key = rootKey.addIndex(0);
    jsonDoc.moveMember(rootKey, key); // Move the original value to the new array index

    // Return whether wrapping succeeded
    auto const newType = jsonDoc.memberType(rootKey);
    return newType == Data::KeyType::array;
}

bool Array::push(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    auto const arraySize = jsonDoc.memberSize(rootKey);
    auto const key = rootKey.addIndex(arraySize);
    jsonDoc.set(key, Utility::StringHandler::recombineArgs(args.subspan(1)));
    return true;
}

bool Array::pushNumber(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        double const number = std::stod(std::string(args.at(1)));
        if (!ensureArray(jsonDoc)) {
            return false;
        }
        auto const arraySize = jsonDoc.memberSize(rootKey);
        auto const key = rootKey.addIndex(arraySize);
        jsonDoc.set(key, number);
        return true;
    } catch (...) {
        return false;
    }
}


bool Array::enumerate(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    if (args.size() < 2) return false;
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) return false;
    auto const& indexKey = args.at(1);
    std::ranges::for_each(
        std::views::iota(std::size_t{0}, jsonDoc.memberSize(rootKey)),
        [&](std::size_t const i) {
            auto const key = rootKey.addIndex(i).addMember(indexKey);
            jsonDoc.set(key, i);
        }
    );
    return true;
}

bool Array::iota(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    if (args.size() < 3) return false;
    auto start = std::stoll(std::string(args.at(1)));
    auto end = std::stoll(std::string(args.at(2)));
    if (start >= end) {
        jsonDoc.setEmptyArray(rootKey);
        return true;
    }
    std::ranges::for_each(
        std::views::iota(start, end),
        [&](auto const& i) {
            auto index = i - start;
            auto const key = rootKey.addIndex(static_cast<size_t>(index));
            jsonDoc.set(key, i);
        }
    );
    return true;
}

} // namespace Nebulite::Module::Transformation
