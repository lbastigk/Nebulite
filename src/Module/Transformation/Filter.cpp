//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <cstdint> // NOLINT
#include <functional>
#include <ranges>
#include <regex>
#include <span>
#include <string>
#include <utility>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Module/Transformation/Filter.hpp"
#include "Nebulite/Utility/Glob.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Filter::bindTransformations(){
    bindTransformation(&Filter::filterRegex, filterRegexName, filterRegexDesc);
    bindTransformation(&Filter::filterGlob, filterGlobName, filterGlobDesc);
    bindTransformation(&Filter::filterRegexValue, filterRegexValueName, filterRegexValueDesc);
    bindTransformation(&Filter::filterGlobValue, filterGlobValueName, filterGlobValueDesc);
    bindTransformation(&Filter::filterNulls, filterOutNullsName, filterOutNullsDesc);
    bindTransformation(&Filter::filterCustom, filterCustomName, filterCustomDesc);
}

bool Filter::filterRegex(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    std::regex regexPattern;
    try {
        std::string const pattern = Utility::StringHandler::recombineArgs(args.subspan(1));
        regexPattern = std::regex(pattern);
    } catch (const std::regex_error&) {
        return false; // Invalid regex pattern
    }

    auto const memberKeyPairs = jsonDoc.listAvailableMembersAndKeys(rootKey);
    Data::JSON filtered;
    for (const auto& [member, key] : memberKeyPairs) {
        if (std::regex_match(member, regexPattern)) {
            filtered.setSubDoc(member, jsonDoc.getSubDoc(key));
        }
    }

    jsonDoc.setSubDoc(rootKey, filtered);
    return true;
}

bool Filter::filterGlob(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    std::string const pattern = Utility::StringHandler::recombineArgs(args.subspan(1));
    auto const memberKeyPairs = jsonDoc.listAvailableMembersAndKeys(rootKey);
    Data::JSON filtered;
    for (const auto& [member, key] : memberKeyPairs) {
        if (Utility::globMatch(pattern, member)) {
            filtered.setSubDoc(member, jsonDoc.getSubDoc(key));
        }
    }
    jsonDoc.setSubDoc(rootKey, filtered);
    return true;
}

bool Filter::filterRegexValue(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    if (args.size() != 2) {
        return false;
    }
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return false; // Not an array, cannot filter values
    }

    std::regex regexPattern;
    try {
        std::string const pattern = Utility::StringHandler::recombineArgs(args.subspan(1));
        regexPattern = std::regex(pattern);
    } catch (const std::regex_error&) {
        return false; // Invalid regex pattern
    }

    // Get values and filter
    auto const values = listMemberValues(jsonDoc, rootKey)
        | std::views::filter(
            [regexPattern](std::string const& value) {
                try {
                    return std::regex_match(value, regexPattern);
                } catch (const std::regex_error&) {
                    return false; // Invalid regex pattern
                }
            })
        | std::ranges::to<std::vector>();

    // Set values
    jsonDoc.setEmptyArray(rootKey);
    for (auto [index, value] : values | std::views::enumerate) {
        auto const key = rootKey.addIndex(static_cast<size_t>(index));
        jsonDoc.set(key, value);
    }
    return true;
}

bool Filter::filterGlobValue(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    if (args.size() != 2) {
        return false;
    }
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return false; // Not an array, cannot filter values
    }
    auto const pattern = Utility::StringHandler::recombineArgs(args.subspan(1));

    // Get values and filter
    auto const values = listMemberValues(jsonDoc, rootKey)
        | std::views::filter([pattern](std::string const& value) { return Utility::globMatch(pattern, value); }) // NOLINT
        | std::ranges::to<std::vector>();

    // Set values
    jsonDoc.setEmptyArray(rootKey);
    for (auto [index, value] : values | std::views::enumerate) {
        auto const key = rootKey.addIndex(static_cast<size_t>(index));
        jsonDoc.set(key, value);
    }
    return true;
}

// NOLINTNEXTLINE
bool Filter::filterNulls(Data::JsonScope& jsonDoc) {
    auto const type = jsonDoc.memberType(rootKey);

    if (type == Data::KeyType::null) {
        jsonDoc.removeMember(rootKey);
        return true;
    }

    // All other types need to be handled recursively
    if (type == Data::KeyType::value) {
        return true; // Single value, nothing to filter
    }

    // For arrays and objects, we need to iterate through members
    auto const rootType = jsonDoc.memberType(rootKey);
    std::size_t arrayIndex = 0;
    auto const memberKeyPairs = jsonDoc.listAvailableMembersAndKeys(rootKey);
    Data::JSON filteredObject;
    for (const auto& [member, key] : memberKeyPairs) {
        auto& memberScope = jsonDoc.shareScope(key);
        filterNulls(memberScope);

        // If the member has no more members, we also remove it
        // This allows us to remove empty objects/arrays: {} and []
        auto const memberType = jsonDoc.memberType(key);
        if (memberType != Data::KeyType::value && jsonDoc.listAvailableKeys(key).empty()) {
            continue;
        }

        auto const memberValue = jsonDoc.getSubDoc(key);
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
    jsonDoc.setSubDoc(rootKey, filteredObject);
    return true;
}

bool Filter::filterCustom(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) return false; // Not an array, cannot sort
    if (args.size() < 2) return false;
    Interaction::Logic::Expression const expression('$' + Utility::StringHandler::recombineArgs(args.subspan(1)));
    arrayFilter(jsonDoc, [&](Data::JsonScope& element) {
        Interaction::ContextScope const ctxScope{
            {
                .self = element,
                .other = element,
                .global = element
            }
        };
        return expression.evalAsBool(ctxScope);
    });
    return true;
}

// Private helper

void Filter::arrayFilter(Data::JsonScope& jsonDoc, std::function<bool(Data::JsonScope&)> const& filter) {
    auto memberCount = jsonDoc.memberSize(rootKey);
    std::vector<Data::JSON> values;
    values.reserve(memberCount);
    for (auto const idx : std::views::iota(std::size_t{0}, memberCount)) {
        auto const key = rootKey.addIndex(idx);
        auto doc = jsonDoc.getSubDoc(key);
        if (auto& scope = doc.shareManagedScope(""); filter(scope)) {
            values.emplace_back(std::move(doc));
        }
    }
    jsonDoc.removeMember(rootKey);
    for (auto [idx, value] : values | std::views::enumerate) {
        auto const key = rootKey.addIndex(static_cast<std::size_t>(idx));
        jsonDoc.setSubDoc(key, value);
    }
}

std::vector<std::string> Filter::listMemberValues(Data::JsonScope const& jsonDoc, Data::ScopedKeyView const& rootKey) {
    auto maxSize = jsonDoc.memberSize(rootKey);
    std::vector<std::string> values;
    values.reserve(maxSize);
    for (auto const index : std::views::iota(std::size_t{0}, maxSize)) {
        auto const key = rootKey.addIndex(index);
        if (auto const value = jsonDoc.get<std::string>(key); value.has_value()) {
            values.emplace_back(value.value());
        }
    }
    return values;
}

} // namespace Nebulite::Module::Transformation
