//------------------------------------------
// Includes

// Standard library
#include <cstddef> // NOLINTTHISLINE
#include <cstdint> // NOLINTTHISLINE
#include <regex>
#include <span>
#include <string>
#include <vector>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Data/Document/KeyType.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Logic/Expression.hpp"
#include "Module/Transformation/Filter.hpp"
#include "Nebulite.hpp"
#include "Utility/Glob.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Filter::bindTransformations(){
    bindTransformation(&Filter::filterRegex, filterRegexName, filterRegexDesc);
    bindTransformation(&Filter::filterGlob, filterGlobName, filterGlobDesc);
    bindTransformation(&Filter::filterNulls, filterOutNullsName, filterOutNullsDesc);
}

bool Filter::filterRegex(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
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

    auto const memberKeyPairs = jsonDoc->listAvailableMembersAndKeys(rootKey);
    Data::JSON filtered;
    for (const auto& [member, key] : memberKeyPairs) {
        if (std::regex_match(member, regexPattern)) {
            filtered.setSubDoc(member, jsonDoc->getSubDoc(key));
        }
    }

    jsonDoc->setSubDoc(rootKey, filtered);
    return true;
}

bool Filter::filterGlob(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    std::string const pattern = Utility::StringHandler::recombineArgs(args.subspan(1));
    auto const memberKeyPairs = jsonDoc->listAvailableMembersAndKeys(rootKey);
    Data::JSON filtered;
    for (const auto& [member, key] : memberKeyPairs) {
        if (Utility::globMatch(pattern, member)) {
            filtered.setSubDoc(member, jsonDoc->getSubDoc(key));
        }
    }
    jsonDoc->setSubDoc(rootKey, filtered);
    return true;
}

// NOLINTNEXTLINE
bool Filter::filterNulls(Data::JsonScope* jsonDoc) {
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

bool Filter::filterCustom(std::span<std::string const> const& args, Data::JsonScope* jsonDoc){
    if (jsonDoc->memberType(rootKey) != Data::KeyType::array) return false; // Not an array, cannot sort
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

} // namespace Nebulite::Module::Transformation
