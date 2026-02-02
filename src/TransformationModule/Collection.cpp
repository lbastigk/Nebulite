//------------------------------------------
// Includes

// Standard library
#include <fnmatch.h>

// Nebulite
#include "Core/JsonScope.hpp"
#include "TransformationModule/Collection.hpp"

//------------------------------------------
namespace Nebulite::TransformationModule {

void Collection::bindTransformations() {
    // BIND_TRANSFORMATION_MEMBER(&Collection::filter, filterName, &filterDesc);
    BIND_TRANSFORMATION_MEMBER(&Collection::map, mapName, mapDesc);
    BIND_TRANSFORMATION_STATIC(&Collection::get, getName, getDesc);
    BIND_TRANSFORMATION_STATIC(&Collection::getMultiple, getMultipleName, getMultipleDesc);
    BIND_TRANSFORMATION_STATIC(&Collection::filterGlob, filterGlobName, filterGlobDesc);
    BIND_TRANSFORMATION_STATIC(&Collection::filterNulls, filterOutNullsName, filterOutNullsDesc);
    BIND_TRANSFORMATION_STATIC(&Collection::listMembers, listKeysName, listKeysDesc);
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
            jsonDoc->removeMember(elementKey);
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

// NOLINTNEXTLINE
bool Collection::filterNulls(Core::JsonScope* jsonDoc) {
    auto const type = jsonDoc->memberType(rootKey);

    if (type == Data::KeyType::null) {
        jsonDoc->removeMember(rootKey);
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
        // This allows us to remove empty objects/arrays: {} and []
        auto const memberType = jsonDoc->memberType(key);
        if (memberType != Data::KeyType::value && jsonDoc->listAvailableKeys(key).empty()) {
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

bool Collection::listMembers(Core::JsonScope* jsonDoc){
    auto const membersAndKeys = jsonDoc->listAvailableMembersAndKeys(rootKey);
    jsonDoc->removeMember(rootKey);
    std::ranges::for_each(
    std::views::zip(std::views::iota(std::size_t{0}), membersAndKeys),
        [&](auto const& enumeratedMemberAndKey) {
            auto const& [i, memberAndKey] = enumeratedMemberAndKey;
            auto const& [member, _] = memberAndKey;

            jsonDoc->set<std::string>(
                rootKey + "[" + std::to_string(i) + "]",
                member
            );
        }
    );
    return true;
}

} // namespace Nebulite::TransformationModule
