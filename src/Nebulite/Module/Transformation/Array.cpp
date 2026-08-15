//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint> // NOLINT
#include <ranges>
#include <span>
#include <string>
#include <utility>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Module/Transformation/Array.hpp"
#include "Nebulite/Utility/Convert/Cast.hpp"
#include "Nebulite/Utility/Ranges.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Array::bindTransformations() {
    // Pick
    bindTransformation(&Array::at, atName, atDesc);
    bindTransformation(&Array::first, firstName, firstDesc);
    bindTransformation(&Array::last, lastName, lastDesc);
    bindTransformation(&Array::length, lengthName, lengthDesc);
    bindTransformation(&Array::subspan, subspanName, subspanDesc);

    // Metadata
    bindTransformation(&Array::flatten, flattenName, flattenDesc);
    bindTransformation(&Array::reverse, reverseName, reverseDesc);
    bindTransformation(&Array::enumerate, enumerateName, enumerateDesc);
    bindTransformation(&Array::batch, batchName, batchDesc);
    bindTransformation(&Array::batchPadded, batchPaddedName, batchPaddedDesc);
    bindTransformation(&Array::stride, strideName, strideDesc);
    bindTransformation(&Array::slide, slideName, slideDesc);

    // Generate
    bindTransformation(&Array::iota, iotaName, iotaDesc);

    // Other
    bindTransformation(&Array::ensureArray, ensureArrayName, ensureArrayDesc);
    bindTransformation(&Array::push, pushName, pushDesc);
    bindTransformation(&Array::pushNumber, pushNumberName, pushNumberDesc);
    bindTransformation(&Array::pad, padName, padDesc);
}

// Pick

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
        Data::Json const temp = jsonDoc.getSubDoc(rootKey.addIndex(index));
        jsonDoc.setSubDoc(rootKey, temp);
        return true;
    } catch (...) {
        return false;
    }
}

bool Array::first(Data::JsonScope& jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    if (jsonDoc.memberSize(rootKey) == 0) {
        return false; // Empty array
    }
    Data::Json const firstElement = jsonDoc.getSubDoc(rootKey.addIndex(0));
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
    Data::Json const lastElement = jsonDoc.getSubDoc(rootKey.addIndex(arraySize - 1));
    jsonDoc.setSubDoc(rootKey, lastElement);
    return true;
}

bool Array::length(Data::JsonScope& jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    auto const len = jsonDoc.memberSize(rootKey);
    jsonDoc.set(rootKey, static_cast<uint64_t>(len));
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

// Metadata

namespace {

/**
 * @brief Inserts elements from a JSON document into a temporary JSON object as a flat array
 * @param tmp The temporary JSON object to insert elements into.
 * @param index The index to use when inserting elements.
 * @param jsonDoc The JSON document to read elements from.
 * @param key The key of the element to read.
 * @param root The root key for the temporary JSON object.
 * @return True if the insertion was successful, false otherwise.
 *         On failure, tmp may contain a partially populated result and
 *         must not be used as a complete output.
 */
bool insertIntoArray(Data::Json& tmp, std::size_t& index, Data::JsonScope const& jsonDoc, Data::ScopedKeyView const& key, Data::ScopedKeyView const& root) {
    switch (jsonDoc.memberType(key)) {
    case Data::KeyType::array:
        for (auto const subKey : jsonDoc.arrayKeys(key)) {
            if (!insertIntoArray(tmp, index, jsonDoc, subKey.view(), root)) {
                return false;
            }
        }
        break;
    case Data::KeyType::object:
        tmp.setSubDoc(root.addIndex(index++).toString(), jsonDoc.getSubDoc(key));
        break;
    case Data::KeyType::value: {
        auto const variant = jsonDoc.getVariant(key);
        if (!variant.has_value()) {
            return false;
        }
        tmp.setVariant(root.addIndex(index++).toString(), variant.value());
        break;
    }
    case Data::KeyType::null:
        return false;
    default:
        std::unreachable();
    }
    return true;
}

[[maybe_unused]] std::size_t calculateRequiredBatchSize(std::size_t arraySize, std::size_t batchSize) {
    if (arraySize % batchSize == 0) {
        return arraySize / batchSize;
    }
    return arraySize / batchSize + 1;
}

[[maybe_unused]] bool allArraysEqualInSize(Data::JsonScope const& jsonDoc, Data::ScopedKeyView const& rootKey, std::size_t expectedSize) {
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return false;
    }
    return std::ranges::all_of(
        jsonDoc.arrayKeys(rootKey),
        [&](Data::ScopedKey const& key) {
            return jsonDoc.memberType(key) == Data::KeyType::array && jsonDoc.memberSize(key) == expectedSize;
        }
    );
}

} // namespace

bool Array::flatten(Data::JsonScope& jsonDoc){
    Data::Json tmp;
    if (std::size_t tmpIndex = 0; !insertIntoArray(tmp, tmpIndex, jsonDoc, rootKey, rootKey)) {
        jsonDoc.setEmptyArray(rootKey);
        return false;
    }
    jsonDoc.setSubDoc(rootKey, tmp);
    return true;
}

bool Array::reverse(Data::JsonScope& jsonDoc) {
    if (!ensureArray(jsonDoc)) {
        return false;
    }
    auto const arraySize = jsonDoc.memberSize(rootKey);
    Data::Json const tmp = jsonDoc.getSubDoc(rootKey);
    for (std::size_t i = 0; i < arraySize; ++i) {
        auto const key = rootKey.addIndex(i);
        auto const elementKey = rootKey.addIndex(arraySize - 1 - i).toString();
        Data::Json const element = tmp.getSubDoc(elementKey);
        jsonDoc.setSubDoc(key, element);
    }
    return true;
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

bool Array::batch(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    // Validate arguments and input
    if (args.size() < 2) return false;
    auto const size = Utility::Convert::Cast::String::to<std::size_t>(args.at(1));
    if (!size.has_value()) return false;
    if (size.value() == 0) return false;
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) return false;
    auto arraySize = jsonDoc.memberSize(rootKey);

    // Edge case: do not modify empty arrays
    if (arraySize == 0) return true;

    // Batch the array into subarrays of the specified size, moving members to their new locations
    // JsonScope::moveMember modifies the array size, so we need to keep track of the batch index separately
    std::size_t oldIndex = 0;
    for (auto const index : std::views::iota(std::size_t{0}, arraySize)) {
        auto const newBatchIndex = index / size.value();
        auto const batchArrayIndex = index % size.value();

        auto const oldKey = rootKey.addIndex(oldIndex);
        auto const newKey = rootKey.addIndex(newBatchIndex).addIndex(batchArrayIndex);
        jsonDoc.moveMember(oldKey, newKey);

        // See if we modified the array size during the move
        // If not, we need to increment the oldIndex to keep up with the next element to move
        if (oldIndex == newBatchIndex) {
            ++oldIndex;
        }
    }

    assert(jsonDoc.memberSize(rootKey) == calculateRequiredBatchSize(arraySize, size.value()));
    return true;
}

bool Array::batchPadded(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
if (!batch(args, jsonDoc)) return false;
auto const size = Utility::Convert::Cast::String::to<std::size_t>(args.at(1));
if (!size.has_value()) return false;
auto const batchCount = jsonDoc.memberSize(rootKey);
if (batchCount == 0) return true;
auto const lastBatch = rootKey.addIndex(batchCount - 1);
while (jsonDoc.memberSize(lastBatch) < size.value()) {
        auto const newIndex = jsonDoc.memberSize(lastBatch);
        auto const newKey = lastBatch.addIndex(newIndex);
        jsonDoc.setEmptyObject(newKey);
    }
    assert(allArraysEqualInSize(jsonDoc, rootKey, size.value()));
    return true;
}

bool Array::stride(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    if (args.size() < 2) return false;
    auto const size = Utility::Convert::Cast::String::to<std::size_t>(args.at(1));
    if (!size.has_value()) return false;
    if (size.value() == 0) return false;
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) return false;
    Data::JsonScope tmp;
    tmp.setEmptyArray(rootKey);
    for (auto const [index, key] : jsonDoc.arrayKeys(rootKey) | std::views::stride(size.value()) | Utility::Ranges::enumerate) {
        tmp.setSubDoc(rootKey.addIndex(index), jsonDoc.getSubDoc(key));
    }
    jsonDoc.setSubDoc(rootKey, tmp);
    return true;
}

bool Array::slide(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    if (args.size() < 2) return false;
    auto const size = Utility::Convert::Cast::String::to<std::size_t>(args.at(1));
    if (!size.has_value()) return false;
    if (size.value() == 0) return false;
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) return false;
    Data::JsonScope tmp;
    tmp.setEmptyArray(rootKey);
    for (auto const [index, keys] : jsonDoc.arrayKeys(rootKey) | std::views::slide(size.value()) | Utility::Ranges::enumerate) {
        for (auto const [subIndex, key] : keys | Utility::Ranges::enumerate) {
            tmp.setSubDoc(rootKey.addIndex(index).addIndex(subIndex), jsonDoc.getSubDoc(key));
        }
    }
    jsonDoc.setSubDoc(rootKey, tmp);
    return true;
}

// Generate

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

// Other

bool Array::ensureArray(Data::JsonScope& jsonDoc) {
    auto type = jsonDoc.memberType(rootKey);

    // Already array, nothing to do
    if (type == Data::KeyType::array) {
        return true;
    }

    // No value stored, set to empty array
    if (type == Data::KeyType::null) {
        jsonDoc.setEmptyArray(rootKey);
        return true;
    }

    // Single value, wrap into an array
    auto const key = rootKey.addIndex(0);
    jsonDoc.moveMember(rootKey, key); // Move the original value to the new array index

    // Return whether wrapping succeeded
    return jsonDoc.memberType(rootKey) == Data::KeyType::array;
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

bool Array::pad(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    if (args.size() < 2) return false;
    auto const size = Utility::Convert::Cast::String::to<std::size_t>(args.at(1));
    if (!size.has_value()) return false;
    if (!ensureArray(jsonDoc)) return false;
    while (jsonDoc.memberSize(rootKey) < size.value()) {
        auto const newIndex = jsonDoc.memberSize(rootKey);
        auto const newKey = rootKey.addIndex(newIndex);
        jsonDoc.setEmptyObject(newKey);
    }
    return true;
}

} // namespace Nebulite::Module::Transformation
