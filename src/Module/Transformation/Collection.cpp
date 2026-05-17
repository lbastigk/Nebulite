//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
// NOLINTNEXTLINE
#include <cstdint>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Data/Document/KeyType.hpp"
#include "Module/Transformation/Collection.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Collection::bindTransformations() {
    bindTransformation(&Collection::map, mapName, mapDesc);
    bindTransformation(&Collection::get, getName, getDesc);
    bindTransformation(&Collection::listMembers, listMembersName, listMembersDesc);
    bindTransformation(&Collection::listMembersAndValues, listMembersAndValuesName, listMembersAndValuesDesc);
    bindTransformation(&Collection::bundleToArray, bundleToArrayName, bundleToArrayDesc);
}

bool Collection::map(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(rootKey) != Data::KeyType::array) {
        auto const key = rootKey.addIndex(0);
        jsonDoc->moveMember(rootKey, key);
    }

    // Now we expect an array
    if (jsonDoc->memberType(rootKey) != Data::KeyType::array) {
        return false; // still not an array, something went wrong
    }

    size_t const arraySize = jsonDoc->memberSize(rootKey);
    for (uint32_t idx = 0; idx < arraySize; ++idx) {
        // Set temp document with current element
        auto const elementKey = rootKey.addIndex(idx);

        // Parse transformation command
        if (auto& scope = jsonDoc->shareScope(elementKey); !scope.transform(args)) {
            jsonDoc->removeMember(elementKey);
            return false; // If parsing fails for any element, we remove it and return false
        }
    }
    return true;
}

bool Collection::get(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    auto const& key = rootKey.addMember(args[1]);
    Data::JSON const subDoc = jsonDoc->getSubDoc(key);
    jsonDoc->setSubDoc(rootKey, subDoc);
    return true;
}

bool Collection::listMembers(Data::JsonScope* jsonDoc){
    auto const membersAndKeys = jsonDoc->listAvailableMembersAndKeys(rootKey);
    jsonDoc->removeMember(rootKey);
    std::ranges::for_each(
    std::views::zip(std::views::iota(std::size_t{0}), membersAndKeys),
        [&](auto const& enumeratedMemberAndKey) {
            auto const& [i, memberAndKey] = enumeratedMemberAndKey;
            auto const& [member, _] = memberAndKey;
            auto key = Data::ScopedKey(rootKey.addIndex(i));
            jsonDoc->set<std::string>(key,member);
        }
    );
    return true;
}

bool Collection::listMembersAndValues(Data::JsonScope* jsonDoc){
    auto const membersAndKeys = jsonDoc->listAvailableMembersAndKeys(rootKey);
    std::vector<Data::JSON> values;
    std::ranges::for_each(
    std::views::zip(std::views::iota(std::size_t{0}), membersAndKeys),
        [&](auto const& enumeratedMemberAndKey) {
            auto const& [i, memberAndKey] = enumeratedMemberAndKey;
            auto const& [member, key] = memberAndKey;
            Data::JSON newObject;
            newObject.deserialize(jsonDoc->serialize(key));
            values.push_back(std::move(newObject));
        }
    );

    jsonDoc->removeMember(rootKey);
    std::ranges::for_each(
    std::views::zip(std::views::iota(std::size_t{0}), membersAndKeys),
        [&](auto const& enumeratedMemberAndKey) {
            auto const& [i, memberAndKey] = enumeratedMemberAndKey;
            auto const& [member, key] = memberAndKey;
            auto key1 = rootKey.addIndex(i).addMember("key");
            auto key2 = rootKey.addIndex(i).addMember("value");
            jsonDoc->set<std::string>(key1,member);
            jsonDoc->setSubDoc(key2,values[i]);
        }
    );
    return true;
}

bool Collection::bundleToArray(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    Data::JSON tmp;
    for (auto [idx, key] : args.subspan(1) | std::views::enumerate) {
        tmp.setSubDoc("[" + std::to_string(idx) + "]", jsonDoc->getSubDoc(rootKey.addMember(key)));
    }
    jsonDoc->setSubDoc(rootKey, tmp);
    return true;
}

} // namespace Nebulite::Module::Transformation
