#include <fnmatch.h>

#include "TransformationModule/Collection.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::TransformationModule {

void Collection::bindTransformations() {
    // BIND_TRANSFORMATION_MEMBER(&Collection::filter, filterName, &filterDesc);
    BIND_TRANSFORMATION_MEMBER(&Collection::map, mapName, mapDesc);
    BIND_TRANSFORMATION_STATIC(&Collection::get, getName, getDesc);
    BIND_TRANSFORMATION_STATIC(&Collection::getMultiple, getMultipleName, getMultipleDesc);
    BIND_TRANSFORMATION_STATIC(&Collection::filterGlob, filterGlobName, filterGlobDesc);
    BIND_TRANSFORMATION_STATIC(&Collection::filterNulls, filterOutNullsName, filterOutNullsDesc);
}

bool Collection::map(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) const {
    if (jsonDoc->memberType(rootKey) == Data::KeyType::value) {
        // Single value, we wrap it into an array
        // TODO: optimize by using moveMember, once available
        Data::JSON const tmp = jsonDoc->getSubDoc(rootKey);
        auto const key = rootKey + "[0]";
        jsonDoc->setSubDoc(key, tmp);
    }
    // Now we expect an array
    if (jsonDoc->memberType(rootKey) != Data::KeyType::array) {
        return false; // Not an array
    }
    // Re-join args into a single transformation command
    std::string cmd = __FUNCTION__;
    for (auto const& arg : args.subspan(1)) {
        cmd += " ";
        cmd += arg;
    }

    size_t const arraySize = jsonDoc->memberSize(rootKey);
    for (uint32_t idx = 0; idx < arraySize; ++idx) {
        // Set temp document with current element
        auto const elementKey = rootKey + "[" + std::to_string(idx) + "]";

        // Parse transformation command
        auto& scope = jsonDoc->shareScope(elementKey);
        if (!transformationFuncTree->parseStr(cmd, &scope)) {
            jsonDoc->removeKey(elementKey);
        }
    }
    return true;
}

bool Collection::get(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    auto const& key = rootKey + args[1];
    Data::JSON const subDoc = jsonDoc->getSubDoc(key);
    jsonDoc->setSubDoc(rootKey, subDoc);
    return true;
}

bool Collection::getMultiple(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    Data::JSON tmp;
    size_t i = 0;
    for (auto const& key : args.subspan(1)) {
        tmp.setSubDoc("[" + std::to_string(i) + "]", jsonDoc->getSubDoc(rootKey + key));
        ++i;
    }

    // Create result array
    jsonDoc->setSubDoc(rootKey, tmp);
    return true;
}

bool Collection::filterGlob(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    std::string const pattern = args[1];

    auto const memberKeyPairs = jsonDoc->listAvailableMembersAndKeys(rootKey);
    Data::JSON filtered;
    for (const auto& [member, key] : memberKeyPairs) {
        if (fnmatch(pattern.c_str(), member.c_str(), 0) == 0) {
            filtered.setSubDoc(member, jsonDoc->getSubDoc(key));
        }
    }

    jsonDoc->setSubDoc(rootKey, filtered);
    return true;
}

bool Collection::filterNulls(Core::JsonScope* jsonDoc) {
    auto const type = jsonDoc->memberType(rootKey);

    if (type == Data::KeyType::null) {
        jsonDoc->removeKey(rootKey);
        return true;
    }

    // All other types need to be handled recursively
    if (type == Data::KeyType::value) {
        return true; // Single value, nothing to filter
    }

    // For arrays and objects, we need to iterate through members
    auto const rootType = jsonDoc->memberType(rootKey);
    size_t arrayIndex = 0;
    auto const memberKeyPairs = jsonDoc->listAvailableMembersAndKeys(rootKey);
    Data::JSON filteredObject;
    for (const auto& [member, key] : memberKeyPairs) {
        auto& memberScope = jsonDoc->shareScope(key);
        filterNulls(&memberScope);

        // If the member has no more members, we also remove it
        auto const memberType = jsonDoc->memberType(key);
        auto const memberKeys = jsonDoc->listAvailableKeys(key);
        if (memberType != Data::KeyType::value && memberKeys.empty()) {
            continue;
        }

        auto const memberValue = jsonDoc->getSubDoc(key);
        if (memberType != Data::KeyType::null) {
            if (rootType == Data::KeyType::array) {
                // For arrays, we need to reindex the keys
                filteredObject.setSubDoc("[" + std::to_string(arrayIndex) + "]", memberValue);
                arrayIndex++;
            } else {
                filteredObject.setSubDoc(member, memberValue);
            }
        }
    }
    jsonDoc->setSubDoc(rootKey, filteredObject);

    return true;
}

} // namespace Nebulite::TransformationModule
